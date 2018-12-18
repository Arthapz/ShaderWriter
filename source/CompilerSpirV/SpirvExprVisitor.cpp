/*
See LICENSE file in root folder
*/
#include "SpirvExprVisitor.hpp"

#include "SpirvHelpers.hpp"
#include "SpirvGetSwizzleComponents.hpp"
#include "SpirvImageAccessConfig.hpp"
#include "SpirvImageAccessNames.hpp"
#include "SpirvIntrinsicConfig.hpp"
#include "SpirvIntrinsicNames.hpp"
#include "SpirvMakeAccessChain.hpp"
#include "SpirvTextureAccessConfig.hpp"
#include "SpirvTextureAccessNames.hpp"

#include <ShaderAST/Type/TypeImage.hpp>
#include <ShaderAST/Type/TypeSampledImage.hpp>

#include <numeric>
#include <sstream>

namespace spirv
{
	namespace
	{
		spv::Id loadVariable( spv::Id varId
			, ast::type::TypePtr type
			, Module & module
			, Block & currentBlock
			, LoadedVariableArray & loadedVariables )
		{
			auto it = std::find_if( loadedVariables.begin()
				, loadedVariables.end()
				, [varId]( LoadedVariable const & lookup )
				{
					return lookup.varId == varId;
				} );

			if ( loadedVariables.end() == it )
			{
				auto loadedRhsId = module.loadVariable( varId
					, type
					, currentBlock );
				loadedVariables.push_back( { varId, loadedRhsId } );
				it = loadedVariables.begin() + loadedVariables.size() - 1u;
			}

			return it->loadedId;
		}

		inline spv::Op getCastOp( ast::type::Kind src, ast::type::Kind dst )
		{
			spv::Op result = spv::OpNop;

			if ( isDoubleType( src ) )
			{
				if ( isFloatType( dst ) )
				{
					result = spv::OpFConvert;
				}
				else if ( isSignedIntType( dst ) )
				{
					result = spv::OpConvertFToS;
				}
				else if ( isUnsignedIntType( dst ) )
				{
					result = spv::OpConvertFToU;
				}
				else
				{
					assert( false && "Unsupported cast expression" );
				}
			}
			else if ( isFloatType( src ) )
			{
				if ( isDoubleType( dst ) )
				{
					result = spv::OpFConvert;
				}
				else if ( isSignedIntType( dst ) )
				{
					result = spv::OpConvertFToS;
				}
				else if ( isUnsignedIntType( dst ) )
				{
					result = spv::OpConvertFToU;
				}
				else
				{
					assert( false && "Unsupported cast expression" );
				}
			}
			else if ( isSignedIntType( src ) )
			{
				if ( isDoubleType( dst )
					|| isFloatType( dst ) )
				{
					result = spv::OpConvertSToF;
				}
				else if ( isSignedIntType( dst ) )
				{
				}
				else if ( isUnsignedIntType( dst ) )
				{
					result = spv::OpBitcast;
				}
				else
				{
					assert( false && "Unsupported cast expression" );
				}
			}
			else if ( isUnsignedIntType( src ) )
			{
				if ( isDoubleType( dst )
					|| isFloatType( dst ) )
				{
					result = spv::OpConvertUToF;
				}
				else if ( isSignedIntType( dst ) )
				{
					result = spv::OpBitcast;
				}
				else if ( isUnsignedIntType( dst ) )
				{
				}
				else
				{
					assert( false && "Unsupported cast expression" );
				}
			}
			else
			{
				assert( false && "Unsupported cast expression" );
			}

			return result;
		}
	}

	spv::StorageClass getStorageClass( ast::var::VariablePtr var )
	{
		var = getOutermost( var );
		spv::StorageClass result = spv::StorageClassFunction;

		if ( var->isUniform() )
		{
			if ( var->isConstant() )
			{
				result = spv::StorageClassUniformConstant;
			}
			else
			{
				result = spv::StorageClassUniform;
			}
		}
		else if ( var->isBuiltin() )
		{
			if ( var->isShaderInput() )
			{
				result = spv::StorageClassInput;
			}
			else if ( var->isShaderOutput() )
			{
				result = spv::StorageClassOutput;
			}
			else
			{
				assert( false && "Unsupported built-in variable storage." );
			}
		}
		else if ( var->isShaderInput() )
		{
			result = spv::StorageClassInput;
		}
		else if ( var->isShaderOutput() )
		{
			result = spv::StorageClassOutput;
		}
		else if ( var->isShaderConstant() )
		{
			result = spv::StorageClassInput;
		}
		else if ( var->isSpecialisationConstant() )
		{
			assert( false && "Specialisation constants shouldn't be processed as variables" );
		}
		else if ( var->isPushConstant() )
		{
			result = spv::StorageClassPushConstant;
		}

		return result;
	}

	spv::Id ExprVisitor::submit( ast::expr::Expr * expr
		, Block & currentBlock
		, Module & module
		, bool loadVariable )
	{
		bool allLiterals{ false };
		LoadedVariableArray loadedVariables;
		return submit( expr
			, currentBlock
			, module
			, allLiterals
			, loadVariable
			, loadedVariables );
	}

	spv::Id ExprVisitor::submit( ast::expr::Expr * expr
		, Block & currentBlock
		, Module & module
		, bool loadVariable
		, LoadedVariableArray & loadedVariables )
	{
		bool allLiterals{ false };
		return submit( expr
			, currentBlock
			, module
			, allLiterals
			, loadVariable
			, loadedVariables );
	}

	spv::Id ExprVisitor::submit( ast::expr::Expr * expr
		, Block & currentBlock
		, Module & module
		, bool & allLiterals
		, bool loadVariable )
	{
		LoadedVariableArray loadedVariables;
		return submit( expr
			, currentBlock
			, module
			, allLiterals
			, loadVariable
			, loadedVariables );
	}

	spv::Id ExprVisitor::submit( ast::expr::Expr * expr
		, Block & currentBlock
		, Module & module
		, bool & allLiterals
		, bool loadVariable
		, LoadedVariableArray & loadedVariables )
	{
		spv::Id result{ 0u };
		ExprVisitor vis{ result
			, currentBlock
			, module
			, allLiterals
			, loadVariable
			, loadedVariables };
		expr->accept( &vis );
		return result;
	}

	ExprVisitor::ExprVisitor( spv::Id & result
		, Block & currentBlock
		, Module & module
		, bool & allLiterals
		, bool loadVariable
		, LoadedVariableArray & loadedVariables )
		: m_result{ result }
		, m_currentBlock{ currentBlock }
		, m_module{ module }
		, m_allLiterals{ allLiterals }
		, m_loadVariable{ loadVariable }
		, m_loadedVariables{ loadedVariables }
	{
	}

	spv::Id ExprVisitor::doSubmit( ast::expr::Expr * expr )
	{
		return submit( expr, m_currentBlock, m_module, m_loadVariable, m_loadedVariables );
	}

	spv::Id ExprVisitor::doSubmit( ast::expr::Expr * expr
		, LoadedVariableArray & loadedVariables )
	{
		return submit( expr, m_currentBlock, m_module, m_loadVariable, loadedVariables );
	}

	spv::Id ExprVisitor::doSubmit( ast::expr::Expr * expr
		, bool loadVariable )
	{
		return submit( expr, m_currentBlock, m_module, loadVariable, m_loadedVariables );
	}

	spv::Id ExprVisitor::doSubmit( ast::expr::Expr * expr
		, bool loadVariable
		, LoadedVariableArray & loadedVariables )
	{
		return submit( expr, m_currentBlock, m_module, loadVariable, loadedVariables );
	}

	spv::Id ExprVisitor::doSubmit( ast::expr::Expr * expr
		, bool & allLiterals
		, bool loadVariable )
	{
		return submit( expr, m_currentBlock, m_module, allLiterals, loadVariable, m_loadedVariables );
	}

	spv::Id ExprVisitor::doSubmit( ast::expr::Expr * expr
		, bool & allLiterals
		, bool loadVariable
		, LoadedVariableArray & loadedVariables )
	{
		return submit( expr, m_currentBlock, m_module, allLiterals, loadVariable, loadedVariables );
	}

	void ExprVisitor::visitAssignmentExpr( ast::expr::Assign * expr )
	{
		m_allLiterals = false;
		auto typeId = m_module.registerType( expr->getType() );

		if ( expr->getLHS()->getKind() == ast::expr::Kind::eSwizzle )
		{
			// For LHS swizzle, the variable must first be loaded,
			// then it must be shuffled with the RHS, to compute the final result.
			auto & swizzle = static_cast< ast::expr::Swizzle const & >( *expr->getLHS() );

			// Process the RHS first, asking for the needed variables to be loaded.
			auto rhsId = doSubmit( expr->getRHS(), true, m_loadedVariables );

			auto lhsOuter = swizzle.getOuterExpr();
			assert( lhsOuter->getKind() == ast::expr::Kind::eIdentifier
				|| isAccessChain( lhsOuter ) );

			if ( swizzle.getSwizzle() <= ast::expr::SwizzleKind::e3 )
			{
				// One component swizzles must be processed separately:
				// - Create an access chain to the selected component.
				//   No load to retrieve the variable ID.
				auto lhsId = getVariableIdNoLoad( lhsOuter );
				//   Load the variable manually.
				auto loadedLhsId = loadVariable( lhsId
					, lhsOuter->getType() );
				//   Register component selection as a literal.
				auto componentId = m_module.registerLiteral( uint32_t( swizzle.getSwizzle() ) );
				//   Register pointer type.
				auto typeId = m_module.registerType( swizzle.getType() );
				auto pointerTypeId = m_module.registerPointerType( typeId
					, getStorageClass( static_cast< ast::expr::Identifier const & >( *lhsOuter ).getVariable() ) );
				//   Create the access chain.
				auto intermediateId = m_module.getIntermediateResult();
				m_currentBlock.instructions.emplace_back( makeInstruction< AccessChainInstruction >( pointerTypeId
					, intermediateId
					, IdList{ lhsId, componentId } ) );
				// - Store the RHS into this access chain.
				m_currentBlock.instructions.emplace_back( makeInstruction< StoreInstruction >( intermediateId, rhsId ) );
				m_result = intermediateId;
			}
			else
			{
				// - No load to retrieve the variable ID.
				auto lhsId = getVariableIdNoLoad( lhsOuter );
				// - Load the variable manually.
				auto loadedLhsId = loadVariable( lhsId
					, expr->getType() );
				// - The resulting shuffle indices will contain the RHS values for wanted LHS components,
				//   and LHS values for the remaining ones.
				auto typeId = m_module.registerType( lhsOuter->getType() );
				auto intermediateId = m_module.getIntermediateResult();
				IdList shuffle;
				shuffle.emplace_back( loadedLhsId );
				shuffle.emplace_back( rhsId );
				auto swizzleComponents = getSwizzleComponents( getComponentCount( lhsOuter->getType()->getKind() )
					, swizzle.getSwizzle() );
				shuffle.insert( shuffle.end()
					, swizzleComponents.begin()
					, swizzleComponents.end() );
				m_currentBlock.instructions.emplace_back( makeInstruction< VectorShuffleInstruction >( typeId
					, intermediateId
					, shuffle ) );
				m_currentBlock.instructions.emplace_back( makeInstruction< StoreInstruction >( lhsId, intermediateId ) );
				m_result = lhsId;
			}
		}
		else
		{
			// No load to retrieve the variable ID.
			auto lhsId = getVariableIdNoLoad( expr->getLHS() );
			// Ask for the needed variables to be loaded.
			auto rhsId = doSubmit( expr->getRHS(), true, m_loadedVariables );
			m_currentBlock.instructions.emplace_back( makeInstruction< StoreInstruction >( lhsId, rhsId ) );
			m_result = lhsId;
		}
	}

	void ExprVisitor::visitOpAssignmentExpr( ast::expr::Assign * expr )
	{
		m_allLiterals = false;
		auto typeId = m_module.registerType( expr->getType() );
		// No load to retrieve the variable ID.
		auto lhsId = getVariableIdNoLoad( expr->getLHS() );
		// Load the variable manually.
		auto loadedLhsId = loadVariable( lhsId
			, expr->getType() );
		// Ask for the needed variables to be loaded.
		auto rhsId = doSubmit( expr->getRHS()
			, true );
		auto intermediateId = writeBinOpExpr( expr->getKind()
			, expr->getLHS()->getType()->getKind()
			, expr->getRHS()->getType()->getKind()
			, typeId
			, loadedLhsId
			, rhsId
			, expr->getLHS()->isSpecialisationConstant() );
		m_currentBlock.instructions.emplace_back( makeInstruction< StoreInstruction >( lhsId, intermediateId ) );
		m_module.putIntermediateResult( intermediateId );
		m_module.putIntermediateResult( loadedLhsId );
		m_result = lhsId;
	}

	void ExprVisitor::visitUnaryExpr( ast::expr::Unary * expr )
	{
		m_allLiterals = false;
		auto operandId = doSubmit( expr->getOperand() );
		auto typeId = m_module.registerType( expr->getType() );
		m_result = m_module.getIntermediateResult();

		if ( expr->isSpecialisationConstant() )
		{
			m_currentBlock.instructions.emplace_back( makeInstruction< SpecConstantOpInstruction >( typeId
				, m_result
				, makeOperands( spv::Id( getUnOpCode( expr->getKind(), expr->getType()->getKind() ) )
					, operandId ) ) );
		}
		else
		{
			m_currentBlock.instructions.emplace_back( makeUnInstruction( typeId
				, m_result
				, expr->getKind()
				, expr->getType()->getKind()
				, operandId ) );
		}
	}

	void ExprVisitor::visitBinaryExpr( ast::expr::Binary * expr )
	{
		m_allLiterals = false;
		auto lhsId = doSubmit( expr->getLHS() );
		auto loadedLhsId = lhsId;
		auto rhsId = doSubmit( expr->getRHS() );
		auto loadedRhsId = rhsId;
		auto typeId = m_module.registerType( expr->getType() );
		m_result = writeBinOpExpr( expr->getKind()
			, expr->getLHS()->getType()->getKind()
			, expr->getRHS()->getType()->getKind()
			, typeId
			, loadedLhsId
			, loadedRhsId
			, expr->isSpecialisationConstant() );
	}

	void ExprVisitor::visitCastExpr( ast::expr::Cast * expr )
	{
		m_allLiterals = false;
		auto operandId = doSubmit( expr->getOperand() );
		auto dstTypeId = m_module.registerType( expr->getType() );
		auto op = getCastOp( expr->getOperand()->getType()->getKind()
			, expr->getType()->getKind() );

		if ( op == spv::OpNop )
		{
			m_result = operandId;
		}
		else
		{
			m_result = m_module.getIntermediateResult();

			if ( expr->isSpecialisationConstant() )
			{
				m_currentBlock.instructions.emplace_back( makeInstruction< SpecConstantOpInstruction >( dstTypeId
					, m_result
					, makeOperands( spv::Id( op )
						, operandId ) ) );
			}
			else
			{
				m_currentBlock.instructions.emplace_back( makeCastInstruction( dstTypeId
					, m_result
					, op
					, operandId ) );
			}
		}
	}

	void ExprVisitor::visitPreDecrementExpr( ast::expr::PreDecrement * expr )
	{
		m_allLiterals = false;
		auto literal = m_module.registerLiteral( 1 );
		auto operandId = doSubmit( expr->getOperand(), true );
		auto typeId = m_module.registerType( expr->getType() );
		m_result = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeBinInstruction( typeId
			, m_result
			, ast::expr::Kind::eMinus
			, expr->getOperand()->getType()->getKind()
			, ast::type::Kind::eInt
			, operandId
			, literal ) );
		operandId = m_module.getNonIntermediate( operandId );
		m_currentBlock.instructions.emplace_back( makeInstruction< StoreInstruction >( operandId, m_result ) );
	}

	void ExprVisitor::visitPreIncrementExpr( ast::expr::PreIncrement * expr )
	{
		m_allLiterals = false;
		auto literal = m_module.registerLiteral( 1 );
		auto operandId = doSubmit( expr->getOperand(), true );
		auto typeId = m_module.registerType( expr->getType() );
		m_result = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeBinInstruction( typeId
			, m_result
			, ast::expr::Kind::eAdd
			, expr->getOperand()->getType()->getKind()
			, ast::type::Kind::eInt
			, operandId
			, literal ) );
		operandId = m_module.getNonIntermediate( operandId );
		m_currentBlock.instructions.emplace_back( makeInstruction< StoreInstruction >( operandId, m_result ) );
	}

	void ExprVisitor::visitPostDecrementExpr( ast::expr::PostDecrement * expr )
	{
		m_allLiterals = false;
		auto literal1 = m_module.registerLiteral( 1 );
		auto literal0 = m_module.registerLiteral( 0 );
		auto operandId = doSubmit( expr->getOperand() );
		auto originId = operandId;
		operandId = loadVariable( operandId
			, expr->getType() );

		auto typeId = m_module.registerType( expr->getType() );
		m_result = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeBinInstruction( typeId
			, m_result
			, ast::expr::Kind::eAdd
			, expr->getOperand()->getType()->getKind()
			, ast::type::Kind::eInt
			, operandId
			, literal0 ) );
		auto operand2 = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeBinInstruction( typeId
			, operand2
			, ast::expr::Kind::eMinus
			, expr->getOperand()->getType()->getKind()
			, ast::type::Kind::eInt
			, operandId
			, literal1 ) );
		m_currentBlock.instructions.emplace_back( makeInstruction< StoreInstruction >( originId, operand2 ) );
	}

	void ExprVisitor::visitPostIncrementExpr( ast::expr::PostIncrement * expr )
	{
		m_allLiterals = false;
		auto literal1 = m_module.registerLiteral( 1 );
		auto literal0 = m_module.registerLiteral( 0 );
		auto operandId = doSubmit( expr->getOperand() );
		auto originId = operandId;
		operandId = loadVariable( operandId
			, expr->getType() );

		auto typeId = m_module.registerType( expr->getType() );
		m_result = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeBinInstruction( typeId
			, m_result
			, ast::expr::Kind::eAdd
			, expr->getOperand()->getType()->getKind()
			, ast::type::Kind::eInt
			, operandId
			, literal0 ) );
		auto operand2 = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeBinInstruction( typeId
			, operand2
			, ast::expr::Kind::eAdd
			, expr->getOperand()->getType()->getKind()
			, ast::type::Kind::eInt
			, operandId
			, literal1 ) );
		m_currentBlock.instructions.emplace_back( makeInstruction< StoreInstruction >( originId, operand2 ) );
	}

	void ExprVisitor::visitUnaryPlusExpr( ast::expr::UnaryPlus * expr )
	{
		m_result = doSubmit( expr );
	}

	void ExprVisitor::visitAddAssignExpr( ast::expr::AddAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitAndAssignExpr( ast::expr::AndAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitAssignExpr( ast::expr::Assign * expr )
	{
		visitAssignmentExpr( expr );
	}

	void ExprVisitor::visitDivideAssignExpr( ast::expr::DivideAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitLShiftAssignExpr( ast::expr::LShiftAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitMinusAssignExpr( ast::expr::MinusAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitModuloAssignExpr( ast::expr::ModuloAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitOrAssignExpr( ast::expr::OrAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitRShiftAssignExpr( ast::expr::RShiftAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitTimesAssignExpr( ast::expr::TimesAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitXorAssignExpr( ast::expr::XorAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitAggrInitExpr( ast::expr::AggrInit * expr )
	{
		auto typeId = m_module.registerType( expr->getType() );
		IdList initialisers;
		bool allLiterals = true;

		for ( auto & init : expr->getInitialisers() )
		{
			initialisers.push_back( doSubmit( init.get(), allLiterals, true ) );
		}

		spv::Id init;

		if ( allLiterals )
		{
			init = m_module.registerLiteral( initialisers, expr->getType() );
		}
		else
		{
			init = m_module.getIntermediateResult();
			m_currentBlock.instructions.emplace_back( makeInstruction< CompositeConstructInstruction >( typeId
				, init
				, initialisers ) );
		}

		if ( expr->getIdentifier() )
		{
			m_result = doSubmit( expr->getIdentifier(), false );
			m_currentBlock.instructions.emplace_back( makeInstruction< StoreInstruction >( m_result, init ) );
			m_module.putIntermediateResult( init );
		}
		else
		{
			assert( allLiterals );
			m_result = init;
		}

		m_allLiterals = false;
	}

	void ExprVisitor::visitArrayAccessExpr( ast::expr::ArrayAccess * expr )
	{
		m_allLiterals = false;
		m_result = makeAccessChain( expr, m_module, m_currentBlock, m_loadedVariables );

		if ( m_loadVariable )
		{
			auto result = loadVariable( m_result
				, expr->getType() );
			m_module.putIntermediateResult( m_result );
			m_result = result;
		}
	}

	void ExprVisitor::visitMbrSelectExpr( ast::expr::MbrSelect * expr )
	{
		m_allLiterals = false;
		m_result = makeAccessChain( expr, m_module, m_currentBlock, m_loadedVariables );

		if ( m_loadVariable )
		{
			auto result = loadVariable( m_result
				, expr->getType() );
			m_module.putIntermediateResult( m_result );
			m_result = result;
		}
	}

	void ExprVisitor::visitCompositeConstructExpr( ast::expr::CompositeConstruct * expr )
	{
		IdList params;
		bool allLiterals = true;
		auto paramsCount = 0u;

		for ( auto & arg : expr->getArgList() )
		{
			bool allLitsInit = true;
			auto id = doSubmit( arg.get(), allLitsInit, m_loadVariable );
			params.push_back( id );
			allLiterals = allLiterals && allLitsInit;
			paramsCount += ast::type::getComponentCount( arg->getType()->getKind() );
		}

		auto retCount = ast::type::getComponentCount( expr->getType()->getKind() )
			* ast::type::getComponentCount( ast::type::getComponentType( expr->getType()->getKind() ) );
		auto typeId = m_module.registerType( expr->getType() );

		if ( paramsCount == 1u && retCount != 1u )
		{
			params.resize( retCount, params.back() );
			paramsCount = retCount;
		}

		if ( allLiterals )
		{
			assert( paramsCount == retCount );
			m_result = m_module.registerLiteral( params, expr->getType() );
		}
		else
		{
			m_result = m_module.getIntermediateResult();
			m_currentBlock.instructions.emplace_back( makeInstruction< CompositeConstructInstruction >( typeId
				, m_result
				, params ) );
		}

		m_allLiterals = m_allLiterals && allLiterals;
	}

	void ExprVisitor::visitFnCallExpr( ast::expr::FnCall * expr )
	{
		IdList params;
		bool allLiterals = true;
		auto type = expr->getFn()->getType();
		assert( type->getKind() == ast::type::Kind::eFunction );
		auto fnType = std::static_pointer_cast< ast::type::Function >( type );
		assert( expr->getArgList().size() == fnType->size() );
		auto it = fnType->begin();

		for ( auto & arg : expr->getArgList() )
		{
			auto save = m_loadVariable;
			m_loadVariable = !( *it )->isOutputParam()
				&& !isStructType( arg->getType()->getKind() )
				&& !isArrayType( arg->getType()->getKind() );
			auto id = doSubmit( arg.get() );
			m_loadVariable = save;
			allLiterals = allLiterals
				&& ( arg->getKind() == ast::expr::Kind::eLiteral );

			if ( ( isImageType( arg->getType()->getKind() )
				|| isSampledImageType( arg->getType()->getKind() )
				|| isSamplerType( arg->getType()->getKind() ) ) )
			{
				auto ident = ast::findIdentifier( arg );

				if ( ident
					&& getStorageClass( ident->getVariable() ) != spv::StorageClassFunction )
				{
					// We must have a variable with function storage class.
					// Hence we create a temporary variable with this storage class,
					// and load the orignal variable into it.
					VariableInfo info;
					info.rvalue = true;
					id = m_module.registerVariable( "temp_" + static_cast< ast::expr::Identifier const & >( *arg ).getVariable()->getName()
						, spv::StorageClassFunction
						, arg->getType()
						, info ).id;
					auto loadedImageId = m_module.loadVariable( id, arg->getType(), m_currentBlock );
					m_currentBlock.instructions.emplace_back( makeInstruction< StoreInstruction >( id, loadedImageId ) );
				}
			}

			params.push_back( id );
			++it;
		}

		auto typeId = m_module.registerType( expr->getType() );
		auto fnId = doSubmit( expr->getFn() );
		params.insert( params.begin(), fnId );
		m_result = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeInstruction< FunctionCallInstruction >( typeId
			, m_result
			, params ) );

		m_allLiterals = m_allLiterals && allLiterals;
	}

	void ExprVisitor::visitIdentifierExpr( ast::expr::Identifier * expr )
	{
		m_allLiterals = false;
		auto var = expr->getVariable();

		if ( var->isMember() )
		{
			m_result = makeAccessChain( expr
				, m_module
				, m_currentBlock
				, m_loadedVariables );

			if ( m_loadVariable )
			{
				auto result = loadVariable( m_result
					, expr->getType() );
				m_module.putIntermediateResult( m_result );
				m_result = result;
			}
		}
		else
		{
			if ( var->isSpecialisationConstant() )
			{
				m_result = m_module.registerVariable( var->getName()
					, spv::StorageClassInput
					, expr->getType()
					, m_info ).id;
			}
			else
			{
				m_result = m_module.registerVariable( var->getName()
					, getStorageClass( var )
					, expr->getType()
					, m_info ).id;
			}

			if ( m_loadVariable
				&& ( var->isLocale()
					|| var->isShaderInput()
					|| var->isShaderOutput()
					|| var->isOutputParam() ) )
			{
				m_result = loadVariable( m_result
					, expr->getType() );
			}
		}
	}

	void ExprVisitor::visitImageAccessCallExpr( ast::expr::ImageAccessCall * expr )
	{
		m_allLiterals = false;
		auto paramType = expr->getArgList()[0]->getType();
		assert( paramType->getKind() == ast::type::Kind::eImage );
		auto imageType = std::static_pointer_cast< ast::type::Image >( paramType );
		auto imageVarId = doSubmit( expr->getArgList()[0].get() );
		auto intermediateId = loadVariable( imageVarId, imageType );
		IdList params;
		params.push_back( intermediateId );

		for ( auto it = expr->getArgList().begin() + 1u; it != expr->getArgList().end(); ++it )
		{
			auto & arg = ( *it );
			auto id = doSubmit( arg.get() );
			params.push_back( id );
		}

		IntrinsicConfig config;
		getSpirVConfig( expr->getImageAccess(), config );
		auto op = getSpirVName( expr->getImageAccess() );
		auto typeId = m_module.registerType( expr->getType() );

		if ( config.needsTexelPointer )
		{
			IdList texelPointerParams;
			uint32_t index = 0u;
			texelPointerParams.push_back( params[index++] );
			texelPointerParams.push_back( params[index++] );

			if ( imageType->getConfig().isMS )
			{
				texelPointerParams.push_back( params[index++] );
			}
			else
			{
				texelPointerParams.push_back( m_module.registerLiteral( 0u ) );
			}

			assert( expr->getArgList()[0]->getKind() == ast::expr::Kind::eIdentifier );
			auto imgParam = static_cast< ast::expr::Identifier const & >( *expr->getArgList()[0] ).getType();
			assert( imgParam->getKind() == ast::type::Kind::eImage );
			auto image = std::static_pointer_cast< ast::type::Image >( imgParam );
			auto sampledId = m_module.registerType( m_module.getCache().getBasicType( image->getConfig().sampledType ) );
			auto pointerTypeId = m_module.registerPointerType( sampledId
				, spv::StorageClassImage );
			auto pointerId = m_module.getIntermediateResult();
			m_currentBlock.instructions.emplace_back( makeInstruction< ImageTexelPointerInstruction >( pointerTypeId
				, pointerId
				, texelPointerParams ) );

			auto scopeId = m_module.registerLiteral( uint32_t( spv::ScopeDevice ) );
			IdList accessParams;
			accessParams.push_back( pointerId );
			accessParams.push_back( scopeId );

			if ( op == spv::OpAtomicCompareExchange )
			{
				auto equalMemorySemanticsId = m_module.registerLiteral( uint32_t( spv::MemorySemanticsImageMemoryMask ) );
				auto unequalMemorySemanticsId = m_module.registerLiteral( uint32_t( spv::MemorySemanticsImageMemoryMask ) );
				accessParams.push_back( equalMemorySemanticsId );
				accessParams.push_back( unequalMemorySemanticsId );
			}
			else
			{
				auto memorySemanticsId = m_module.registerLiteral( uint32_t( spv::MemorySemanticsImageMemoryMask ) );
				accessParams.push_back( memorySemanticsId );
			}

			if ( params.size() > index )
			{
				accessParams.insert( accessParams.end()
					, params.begin() + index
					, params.end() );
			}

			m_result = m_module.getIntermediateResult();
			m_currentBlock.instructions.emplace_back( makeImageAccessInstruction( typeId
				, m_result
				, op
				, accessParams ) );
			m_module.putIntermediateResult( pointerId );
		}
		else
		{
			m_result = m_module.getIntermediateResult();
			m_currentBlock.instructions.emplace_back( makeImageAccessInstruction( typeId
				, m_result
				, op
				, params ) );
		}
	}

	void ExprVisitor::visitInitExpr( ast::expr::Init * expr )
	{
		m_allLiterals = false;
		m_module.registerType( expr->getType() );
		auto init = doSubmit( expr->getInitialiser() );
		m_info.lvalue = true;
		m_result = m_module.registerVariable( expr->getIdentifier()->getVariable()->getName()
			, ( ( expr->getIdentifier()->getVariable()->isLocale() || expr->getIdentifier()->getVariable()->isParam() )
				? spv::StorageClassFunction
				: spv::StorageClassUniformConstant )
			, expr->getType()
			, m_info ).id;
		m_currentBlock.instructions.emplace_back( makeInstruction< StoreInstruction >( m_result, init ) );
	}

	void ExprVisitor::visitIntrinsicCallExpr( ast::expr::IntrinsicCall * expr )
	{
		m_allLiterals = false;
		IntrinsicConfig config;
		getSpirVConfig( expr->getIntrinsic(), config );
		auto opCodeId = getSpirVName( expr->getIntrinsic() );

		if ( !config.isExtension )
		{
			auto opCode = spv::Op( opCodeId );

			if ( config.isAtomic )
			{
				handleAtomicIntrinsicCallExpr( opCode, expr );
			}
			else if ( opCode == spv::OpIAddCarry
				|| opCode == spv::OpISubBorrow )
			{
				handleCarryBorrowIntrinsicCallExpr( opCode, expr );
			}
			else if ( opCode == spv::OpUMulExtended
				|| opCode == spv::OpSMulExtended )
			{
				handleMulExtendedIntrinsicCallExpr( opCode, expr );
			}
			else
			{
				handleOtherIntrinsicCallExpr( opCode, expr );
			}
		}
		else
		{
			handleExtensionIntrinsicCallExpr( opCodeId, expr );
		}
	}

	void ExprVisitor::visitLiteralExpr( ast::expr::Literal * expr )
	{
		switch ( expr->getLiteralType() )
		{
		case ast::expr::LiteralType::eBool:
			m_result = m_module.registerLiteral( expr->getValue< ast::expr::LiteralType::eBool >() );
			break;
		case ast::expr::LiteralType::eInt:
			m_result = m_module.registerLiteral( expr->getValue< ast::expr::LiteralType::eInt >() );
			break;
		case ast::expr::LiteralType::eUInt:
			m_result = m_module.registerLiteral( expr->getValue< ast::expr::LiteralType::eUInt >() );
			break;
		case ast::expr::LiteralType::eFloat:
			m_result = m_module.registerLiteral( expr->getValue< ast::expr::LiteralType::eFloat >() );
			break;
		case ast::expr::LiteralType::eDouble:
			m_result = m_module.registerLiteral( expr->getValue< ast::expr::LiteralType::eDouble >() );
			break;
		default:
			assert( false && "Unsupported literal type" );
			break;
		}
	}

	void ExprVisitor::visitQuestionExpr( ast::expr::Question * expr )
	{
		m_allLiterals = false;
		auto ctrlId = doSubmit( expr->getCtrlExpr() );
		auto trueId = doSubmit( expr->getTrueExpr() );
		auto falseId = doSubmit( expr->getFalseExpr() );
		auto type = m_module.registerType( expr->getType() );
		m_result = m_module.getIntermediateResult();
		auto branches = makeOperands( ctrlId, trueId, falseId );

		if ( expr->getCtrlExpr()->isSpecialisationConstant() )
		{
			m_currentBlock.instructions.emplace_back( makeInstruction< SpecConstantOpInstruction >( type
				, m_result
				, makeOperands( spv::Id( spv::OpSelect ), branches ) ) );
		}
		else
		{
			m_currentBlock.instructions.emplace_back( makeInstruction< SelectInstruction >( type
				, m_result
				, branches ) );
		}
	}

	void ExprVisitor::visitSwitchCaseExpr( ast::expr::SwitchCase * expr )
	{
		m_result = doSubmit( expr->getLabel() );
	}

	void ExprVisitor::visitSwitchTestExpr( ast::expr::SwitchTest * expr )
	{
		m_result = doSubmit( expr->getValue() );
	}

	void ExprVisitor::visitSwizzleExpr( ast::expr::Swizzle * expr )
	{
		m_allLiterals = false;
		auto outerId = doSubmit( expr->getOuterExpr() );
		auto rawTypeId = m_module.registerType( expr->getType() );
		auto pointerTypeId = m_module.registerPointerType( rawTypeId, spv::StorageClassFunction );
		bool needsLoad = false;
		m_result = writeShuffle( m_module, m_currentBlock, pointerTypeId, rawTypeId, outerId, expr->getSwizzle(), needsLoad );
	}

	void ExprVisitor::visitTextureAccessCallExpr( ast::expr::TextureAccessCall * expr )
	{
		m_allLiterals = false;
		IdList args;

		for ( auto & arg : expr->getArgList() )
		{
			args.emplace_back( doSubmit( arg.get() ) );
		}

		auto typeId = m_module.registerType( expr->getType() );
		auto kind = expr->getTextureAccess();
		IntrinsicConfig config;
		getSpirVConfig( kind, config );
		auto op = getSpirVName( kind );

		// Load the sampled image variable
		auto sampledImageType = expr->getArgList()[0]->getType();
		assert( sampledImageType->getKind() == ast::type::Kind::eSampledImage );
		auto sampleImageId = args[0];
		auto loadedSampleImageId = loadVariable( sampleImageId, sampledImageType );
		args[0] = loadedSampleImageId;

		if ( config.needsImage )
		{
			// We need to extract the image from the sampled image, to give it to the final instruction.
			auto imageType = std::static_pointer_cast< ast::type::SampledImage >( sampledImageType )->getImageType();
			auto imageTypeId = m_module.registerType( imageType );
			auto imageId = m_module.getIntermediateResult();
			m_currentBlock.instructions.emplace_back( makeInstruction< ImageInstruction >( imageTypeId
				, imageId
				, loadedSampleImageId ) );
			args[0] = imageId;
		}

		if ( config.imageOperandsIndex )
		{
			assert( args.size() >= config.imageOperandsIndex );
			auto mask = getMask( kind );
			args.insert( args.begin() + config.imageOperandsIndex, spv::Id( mask ) );
		}

		m_result = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeTextureAccessInstruction( typeId
			, m_result
			, op
			, args ) );
	}

	void ExprVisitor::handleCarryBorrowIntrinsicCallExpr( spv::Op opCode, ast::expr::IntrinsicCall * expr )
	{
		// Arg 1 is lhs.
		// Arg 2 is rhs.
		// Arg 3 is carry or borrow.
		assert( expr->getArgList().size() == 3u );
		IdList params;
		params.push_back( doSubmit( expr->getArgList()[0].get() ) );
		params.push_back( doSubmit( expr->getArgList()[1].get() ) );

		auto resultStructTypeId = getUnsignedExtendedResultTypeId( isVectorType( expr->getType()->getKind() )
			? getComponentCount( expr->getType()->getKind() )
			: 1 );
		auto resultCarryBorrowId = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeIntrinsicInstruction( resultStructTypeId
			, resultCarryBorrowId
			, opCode
			, params ) );

		auto & carryBorrowArg = *expr->getArgList()[2];
		auto carryBorrowTypeId = m_module.registerType( carryBorrowArg.getType() );
		auto intermediateId = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeInstruction< CompositeExtractInstruction >( carryBorrowTypeId
			, intermediateId
			, IdList{ resultCarryBorrowId, 1u } ) );
		auto carryBorrowId = getVariableIdNoLoad( &carryBorrowArg );
		m_currentBlock.instructions.emplace_back( makeInstruction< StoreInstruction >( carryBorrowId, intermediateId ) );

		auto resultTypeId = m_module.registerType( expr->getType() );
		m_result = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeInstruction< CompositeExtractInstruction >( resultTypeId
			, m_result
			, IdList{ resultCarryBorrowId, 0u } ) );

		m_module.putIntermediateResult( intermediateId );
	}

	void ExprVisitor::handleMulExtendedIntrinsicCallExpr( spv::Op opCode, ast::expr::IntrinsicCall * expr )
	{
		// Arg 1 is lhs.
		// Arg 2 is rhs.
		// Arg 3 is msb.
		// Arg 4 is lsb.
		assert( expr->getArgList().size() == 4u );
		IdList params;
		params.push_back( doSubmit( expr->getArgList()[0].get() ) );
		params.push_back( doSubmit( expr->getArgList()[1].get() ) );
		spv::Id resultStructTypeId;
		auto paramType = expr->getArgList()[0]->getType()->getKind();

		if ( isSignedIntType( paramType ) )
		{
			resultStructTypeId = getSignedExtendedResultTypeId( isVectorType( paramType )
				? getComponentCount( paramType )
				: 1 );
		}
		else
		{
			resultStructTypeId = getUnsignedExtendedResultTypeId( isVectorType( paramType )
				? getComponentCount( paramType )
				: 1 );
		}

		auto resultMulExtendedId = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeIntrinsicInstruction( resultStructTypeId
			, resultMulExtendedId
			, opCode
			, params ) );

		auto & msbArg = *expr->getArgList()[2];
		auto msbTypeId = m_module.registerType( msbArg.getType() );
		auto intermediateMsb = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeInstruction< CompositeExtractInstruction >( msbTypeId
			, intermediateMsb
			, IdList{ resultMulExtendedId, 1u } ) );
		auto msbId = getVariableIdNoLoad( &msbArg );
		m_currentBlock.instructions.emplace_back( makeInstruction< StoreInstruction >( msbId, intermediateMsb ) );

		auto & lsbArg = *expr->getArgList()[3];
		auto lsbTypeId = m_module.registerType( lsbArg.getType() );
		auto intermediateLsb = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeInstruction< CompositeExtractInstruction >( lsbTypeId
			, intermediateLsb
			, IdList{ resultMulExtendedId, 0u } ) );
		auto lsbId = getVariableIdNoLoad( &lsbArg );
		m_currentBlock.instructions.emplace_back( makeInstruction< StoreInstruction >( lsbId, intermediateLsb ) );

		m_module.putIntermediateResult( intermediateMsb );
		m_module.putIntermediateResult( intermediateLsb );
		m_module.putIntermediateResult( resultMulExtendedId );
	}

	void ExprVisitor::handleAtomicIntrinsicCallExpr( spv::Op opCode, ast::expr::IntrinsicCall * expr )
	{
		IdList params;
		auto save = m_loadVariable;
		m_loadVariable = false;
		params.push_back( doSubmit( expr->getArgList()[0].get() ) );
		m_loadVariable = save;

		for ( auto i = 1u; i < expr->getArgList().size(); ++i )
		{
			params.push_back( doSubmit( expr->getArgList()[i].get() ) );
		}

		auto typeId = m_module.registerType( expr->getType() );
		auto scopeId = m_module.registerLiteral( uint32_t( spv::ScopeDevice ) );
		auto memorySemanticsId = m_module.registerLiteral( uint32_t( spv::MemorySemanticsAcquireReleaseMask ) );
		uint32_t index{ 1u };
		params.insert( params.begin() + ( index++ ), scopeId );
		params.insert( params.begin() + ( index++ ), memorySemanticsId );
		m_result = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeIntrinsicInstruction( typeId
			, m_result
			, opCode
			, params ) );
	}

	void ExprVisitor::handleExtensionIntrinsicCallExpr( spv::Id opCode, ast::expr::IntrinsicCall * expr )
	{
		auto intrinsic = expr->getIntrinsic();
		IdList params;

		if ( ( intrinsic >= ast::expr::Intrinsic::eModf1F
				&& intrinsic <= ast::expr::Intrinsic::eModf4D )
			|| ( intrinsic >= ast::expr::Intrinsic::eFrexp1F
				&& intrinsic <= ast::expr::Intrinsic::eFrexp4D ) )
		{
			// For modf and frexp intrinsics, second parameter is an output parameter,
			// hence we need to pass it as a pointer to the call.
			assert( expr->getArgList().size() == 2u );
			params.push_back( doSubmit( expr->getArgList()[0].get() ) );
			auto save = m_loadVariable;
			m_loadVariable = false;
			params.push_back( doSubmit( expr->getArgList()[1].get() ) );
			m_loadVariable = save;
		}
		else
		{
			for ( auto & arg : expr->getArgList() )
			{
				auto id = doSubmit( arg.get() );
				params.push_back( id );
			}
		}

		auto typeId = m_module.registerType( expr->getType() );
		params.insert( params.begin(), opCode );
		params.insert( params.begin(), 1u );
		m_result = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeInstruction< ExtInstInstruction >( typeId
			, m_result
			, params ) );
	}

	void ExprVisitor::handleOtherIntrinsicCallExpr( spv::Op opCode, ast::expr::IntrinsicCall * expr )
	{
		IdList params;

		for ( auto & arg : expr->getArgList() )
		{
			auto id = doSubmit( arg.get() );
			params.push_back( id );
		}

		auto typeId = m_module.registerType( expr->getType() );
		m_result = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeIntrinsicInstruction( typeId
			, m_result
			, opCode
			, params ) );
	}

	spv::Id ExprVisitor::getUnsignedExtendedResultTypeId( uint32_t count )
	{
		--count;

		if ( !m_unsignedExtendedTypes[count] )
		{
			std::string name = "SDW_ExtendedResultTypeU" + std::to_string( count + 1u );
			m_unsignedExtendedTypes[count] = m_module.getCache().getStruct( ast::type::MemoryLayout::eStd430, name );
			auto type = count == 3
				? m_module.getCache().getVec4U()
				: ( count == 2
					? m_module.getCache().getVec3U()
					: ( count == 1
						? m_module.getCache().getVec2U()
						: ( m_module.getCache().getUInt() ) ) );

			if ( m_unsignedExtendedTypes[count]->empty() )
			{
				m_unsignedExtendedTypes[count]->declMember( "result", type );
				m_unsignedExtendedTypes[count]->declMember( "extended", type );
			}
		}

		return m_module.registerType( m_unsignedExtendedTypes[count] );
	}

	spv::Id ExprVisitor::getSignedExtendedResultTypeId( uint32_t count )
	{
		--count;

		if ( !m_signedExtendedTypes[count] )
		{
			std::string name = "SDW_ExtendedResultTypeS" + std::to_string( count + 1u );
			m_signedExtendedTypes[count] = m_module.getCache().getStruct( ast::type::MemoryLayout::eStd430, name );
			auto type = count == 3
				? m_module.getCache().getVec4I()
				: ( count == 2
					? m_module.getCache().getVec3I()
					: ( count == 1
						? m_module.getCache().getVec2I()
						: ( m_module.getCache().getInt() ) ) );

			if ( m_signedExtendedTypes[count]->empty() )
			{
				m_signedExtendedTypes[count]->declMember( "result", type );
				m_signedExtendedTypes[count]->declMember( "extended", type );
			}
		}

		return m_module.registerType( m_signedExtendedTypes[count] );
	}

	spv::Id ExprVisitor::getVariableIdNoLoad( ast::expr::Expr * expr )
	{
		spv::Id result;

		if ( isAccessChain( expr ) )
		{
			auto save = m_loadVariable;
			m_loadVariable = false;
			result = doSubmit( expr );
			m_loadVariable = save;
		}
		else
		{
			auto ident = sdw::findIdentifier( expr );

			if ( ident )
			{
				auto var = ident->getVariable();

				if ( var->isSpecialisationConstant() )
				{
					m_info.lvalue = false;
					m_info.rvalue = true;
					result = m_module.registerVariable( var->getName()
						, spv::StorageClassInput
						, expr->getType()
						, m_info ).id;
				}
				else
				{
					result = m_module.registerVariable( var->getName()
						, getStorageClass( var )
						, expr->getType()
						, m_info ).id;
				}
			}
			else
			{
				result = doSubmit( expr );
			}
		}

		return result;
	}

	spv::Id ExprVisitor::writeBinOpExpr( ast::expr::Kind exprKind
		, ast::type::Kind lhsTypeKind
		, ast::type::Kind rhsTypeKind
		, spv::Id typeId
		, spv::Id lhsId
		, spv::Id rhsId
		, bool isLhsSpecConstant )
	{
		auto result = m_module.getIntermediateResult();

		if ( isLhsSpecConstant )
		{
			m_currentBlock.instructions.emplace_back( makeInstruction< SpecConstantOpInstruction >( typeId
				, result
				, makeBinOpOperands( exprKind
					, lhsTypeKind
					, rhsTypeKind
					, lhsId
					, rhsId ) ) );
		}
		else
		{
			m_currentBlock.instructions.emplace_back( makeBinInstruction( typeId
				, result
				, exprKind
				, lhsTypeKind
				, rhsTypeKind
				, lhsId
				, rhsId ) );
		}

		return result;
	}

	spv::Id ExprVisitor::loadVariable( spv::Id varId
		, ast::type::TypePtr type )
	{
		return spirv::loadVariable( varId
			, type
			, m_module
			, m_currentBlock
			, m_loadedVariables );
	}
}
