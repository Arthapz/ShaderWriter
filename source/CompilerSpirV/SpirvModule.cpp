/*
See LICENSE file in root folder
*/
#include "CompilerSpirV/SpirvModule.hpp"

#include "SpirvHelpers.hpp"

#include <ShaderAST/Type/TypeImage.hpp>
#include <ShaderAST/Type/TypeSampledImage.hpp>
#include <ShaderAST/Type/TypeArray.hpp>

#include <algorithm>

namespace spirv
{
	//*************************************************************************

	namespace
	{
		UInt32List deserializePackedName( UInt32ListCIt & buffer
			, uint32_t & index )
		{
			auto popValue = [&buffer, &index]()
			{
				auto result = *buffer;
				++buffer;
				++index;
				return result;
			};
			auto value = popValue();
			UInt32List result;

			while ( ( value & 0xFF000000 ) != 0u )
			{
				result.push_back( value );
				value = popValue();
			}

			result.push_back( value );
			return result;
		}

		std::vector< uint32_t > const & packString( std::string const & name )
		{
			static std::map < std::string, std::vector< uint32_t > > cache;
			auto it = cache.find( name );

			if ( it == cache.end() )
			{
				std::vector< uint32_t > packed;
				uint32_t word{ 0u };
				uint32_t offset{ 0u };
				size_t i = 0u;

				while ( i < name.size() )
				{
					if ( i != 0 && ( i % 4u ) == 0u )
					{
						packed.push_back( word );
						word = 0u;
						offset = 0u;
					}

					word |= ( uint32_t( name[i] ) & 0x000000FF ) << offset;
					++i;
					offset += 8u;
				}

				if ( word )
				{
					packed.push_back( word );
				}

				if ( i != 0 && ( i % 4u ) == 0u )
				{
					packed.push_back( 0u );
				}

				it = cache.emplace( name, packed ).first;
			}

			return it->second;
		}

		std::string unpackString( std::vector< uint32_t > const & packed )
		{
			std::string result;

			for ( auto w : packed )
			{
				for ( uint32_t j = 0; j < 4; j++, w >>= 8 )
				{
					char c = w & 0xff;

					if ( c == '\0' )
					{
						return result;
					}

					result += c;
				}
			}

			assert( false && "Non terminated string" );
			return std::string{};
		}

		spv::BuiltIn getBuiltin( std::string const & name )
		{
			auto result = spv::BuiltIn( -1 );

			if ( name == "gl_Position" )
			{
				result = spv::BuiltInPosition;
			}
			else if ( name == "gl_PointSize" )
			{
				result = spv::BuiltInPointSize;
			}
			else if ( name == "gl_ClipDistance" )
			{
				result = spv::BuiltInClipDistance;
			}
			else if ( name == "gl_CullDistance" )
			{
				result = spv::BuiltInCullDistance;
			}
			else if ( name == "gl_VertexID" )
			{
				result = spv::BuiltInVertexId;
			}
			else if ( name == "gl_InstanceID" )
			{
				result = spv::BuiltInInstanceId;
			}
			else if ( name == "gl_PrimitiveID" )
			{
				result = spv::BuiltInPrimitiveId;
			}
			else if ( name == "gl_InvocationID" )
			{
				result = spv::BuiltInInvocationId;
			}
			else if ( name == "gl_Layer" )
			{
				result = spv::BuiltInLayer;
			}
			else if ( name == "gl_ViewportIndex" )
			{
				result = spv::BuiltInViewportIndex;
			}
			else if ( name == "gl_TessLevelOuter" )
			{
				result = spv::BuiltInTessLevelOuter;
			}
			else if ( name == "gl_TessLevelInner" )
			{
				result = spv::BuiltInTessLevelInner;
			}
			else if ( name == "gl_TessCoord" )
			{
				result = spv::BuiltInTessCoord;
			}
			else if ( name == "gl_PatchVertices" )
			{
				result = spv::BuiltInPatchVertices;
			}
			else if ( name == "gl_FragCoord" )
			{
				result = spv::BuiltInFragCoord;
			}
			else if ( name == "gl_PointCoord" )
			{
				result = spv::BuiltInPointCoord;
			}
			else if ( name == "gl_FrontFacing" )
			{
				result = spv::BuiltInFrontFacing;
			}
			else if ( name == "gl_SampleID" )
			{
				result = spv::BuiltInSampleId;
			}
			else if ( name == "gl_SamplePosition" )
			{
				result = spv::BuiltInSamplePosition;
			}
			else if ( name == "gl_SampleMask" )
			{
				result = spv::BuiltInSampleMask;
			}
			else if ( name == "gl_FragDepth" )
			{
				result = spv::BuiltInFragDepth;
			}
			else if ( name == "gl_HelperInvocation" )
			{
				result = spv::BuiltInHelperInvocation;
			}
			else if ( name == "gl_NumWorkgroups" )
			{
				result = spv::BuiltInNumWorkgroups;
			}
			else if ( name == "gl_WorkgroupSize" )
			{
				result = spv::BuiltInWorkgroupSize;
			}
			else if ( name == "gl_WorkgroupID" )
			{
				result = spv::BuiltInWorkgroupId;
			}
			else if ( name == "gl_LocalInvocationID" )
			{
				result = spv::BuiltInLocalInvocationId;
			}
			else if ( name == "gl_GlobalInvocationID" )
			{
				result = spv::BuiltInGlobalInvocationId;
			}
			else if ( name == "gl_LocalInvocationIndex" )
			{
				result = spv::BuiltInLocalInvocationIndex;
			}
			else if ( name == "gl_WorkDim" )
			{
				result = spv::BuiltInWorkDim;
			}
			else if ( name == "gl_GlobalSize" )
			{
				result = spv::BuiltInGlobalSize;
			}
			else if ( name == "gl_EnqueuedWorkgroupSize" )
			{
				result = spv::BuiltInEnqueuedWorkgroupSize;
			}
			else if ( name == "gl_GlobalOffset" )
			{
				result = spv::BuiltInGlobalOffset;
			}
			else if ( name == "gl_GlobalLinearID" )
			{
				result = spv::BuiltInGlobalLinearId;
			}
			else if ( name == "gl_SubgroupSize" )
			{
				result = spv::BuiltInSubgroupSize;
			}
			else if ( name == "gl_SubgroupMaxSize" )
			{
				result = spv::BuiltInSubgroupMaxSize;
			}
			else if ( name == "gl_NumSubgroups" )
			{
				result = spv::BuiltInNumSubgroups;
			}
			else if ( name == "gl_NumEnqueuedSubgroups" )
			{
				result = spv::BuiltInNumEnqueuedSubgroups;
			}
			else if ( name == "gl_SubgroupID" )
			{
				result = spv::BuiltInSubgroupId;
			}
			else if ( name == "gl_SubgroupLocalInvocationID" )
			{
				result = spv::BuiltInSubgroupLocalInvocationId;
			}
			else if ( name == "gl_VertexIndex" )
			{
				result = spv::BuiltInVertexIndex;
			}
			else if ( name == "gl_InstanceIndex" )
			{
				result = spv::BuiltInInstanceIndex;
			}
			else if ( name == "gl_SubgroupEqMaskKHR" )
			{
				result = spv::BuiltInSubgroupEqMaskKHR;
			}
			else if ( name == "gl_SubgroupGeMaskKHR" )
			{
				result = spv::BuiltInSubgroupGeMaskKHR;
			}
			else if ( name == "gl_SubgroupGtMaskKHR" )
			{
				result = spv::BuiltInSubgroupGtMaskKHR;
			}
			else if ( name == "gl_SubgroupLeMaskKHR" )
			{
				result = spv::BuiltInSubgroupLeMaskKHR;
			}
			else if ( name == "gl_SubgroupLtMaskKHR" )
			{
				result = spv::BuiltInSubgroupLtMaskKHR;
			}
			else if ( name == "gl_BaseVertex" )
			{
				result = spv::BuiltInBaseVertex;
			}
			else if ( name == "gl_BaseInstance" )
			{
				result = spv::BuiltInBaseInstance;
			}
			else if ( name == "gl_DrawIndex" )
			{
				result = spv::BuiltInDrawIndex;
			}
			else if ( name == "gl_DeviceIndex" )
			{
				result = spv::BuiltInDeviceIndex;
			}
			else if ( name == "gl_ViewIndex" )
			{
				result = spv::BuiltInViewIndex;
			}
			else if ( name == "gl_BaryCoordNoPerspAMD" )
			{
				result = spv::BuiltInBaryCoordNoPerspAMD;
			}
			else if ( name == "gl_BaryCoordNoPerspCentroidAMD" )
			{
				result = spv::BuiltInBaryCoordNoPerspCentroidAMD;
			}
			else if ( name == "gl_BaryCoordNoPerspSampleAMD" )
			{
				result = spv::BuiltInBaryCoordNoPerspSampleAMD;
			}
			else if ( name == "gl_BaryCoordSmoothAMD" )
			{
				result = spv::BuiltInBaryCoordSmoothAMD;
			}
			else if ( name == "gl_BaryCoordSmoothCentroidAMD" )
			{
				result = spv::BuiltInBaryCoordSmoothCentroidAMD;
			}
			else if ( name == "gl_BaryCoordSmoothSampleAMD" )
			{
				result = spv::BuiltInBaryCoordSmoothSampleAMD;
			}
			else if ( name == "gl_BaryCoordPullModelAMD" )
			{
				result = spv::BuiltInBaryCoordPullModelAMD;
			}
			else if ( name == "gl_FragStencilRefEXT" )
			{
				result = spv::BuiltInFragStencilRefEXT;
			}
			else if ( name == "gl_ViewportMaskNV" )
			{
				result = spv::BuiltInViewportMaskNV;
			}
			else if ( name == "gl_SecondaryPositionNV" )
			{
				result = spv::BuiltInSecondaryPositionNV;
			}
			else if ( name == "gl_SecondaryViewportMaskNV" )
			{
				result = spv::BuiltInSecondaryViewportMaskNV;
			}
			else if ( name == "gl_PositionPerViewNV" )
			{
				result = spv::BuiltInPositionPerViewNV;
			}
			else if ( name == "gl_ViewportMaskPerViewNV" )
			{
				result = spv::BuiltInViewportMaskPerViewNV;
			}

			return result;
		}

		ast::type::Kind getComponentType( ast::type::ImageFormat format )
		{
			ast::type::Kind result;

			switch ( format )
			{
			case ast::type::ImageFormat::eRgba32f:
			case ast::type::ImageFormat::eRgba16f:
			case ast::type::ImageFormat::eRg32f:
			case ast::type::ImageFormat::eRg16f:
			case ast::type::ImageFormat::eR32f:
			case ast::type::ImageFormat::eR16f:
				return ast::type::Kind::eFloat;

			case ast::type::ImageFormat::eRgba32i:
			case ast::type::ImageFormat::eRgba16i:
			case ast::type::ImageFormat::eRgba8i:
			case ast::type::ImageFormat::eRg32i:
			case ast::type::ImageFormat::eRg16i:
			case ast::type::ImageFormat::eRg8i:
			case ast::type::ImageFormat::eR32i:
			case ast::type::ImageFormat::eR16i:
			case ast::type::ImageFormat::eR8i:
				return ast::type::Kind::eInt;

			case ast::type::ImageFormat::eRgba32u:
			case ast::type::ImageFormat::eRgba16u:
			case ast::type::ImageFormat::eRgba8u:
			case ast::type::ImageFormat::eRg32u:
			case ast::type::ImageFormat::eRg16u:
			case ast::type::ImageFormat::eRg8u:
			case ast::type::ImageFormat::eR32u:
			case ast::type::ImageFormat::eR16u:
			case ast::type::ImageFormat::eR8u:
				return ast::type::Kind::eUInt;

			default:
				assert( false && "Unsupported ast::type::ImageFormat." );
				return ast::type::Kind::eFloat;
			}

			return result;
		}

		ast::type::TypePtr getUnqualifiedType( ast::type::TypesCache & cache
			, ast::type::TypePtr qualified );

		ast::type::StructPtr getUnqualifiedType( ast::type::TypesCache & cache
			, ast::type::Struct const & qualified )
		{
			auto result = cache.getStruct( qualified.getMemoryLayout(), qualified.getName() );
			assert( result->empty() || ( result->size() == qualified.size() ) );

			if ( result->empty() && !qualified.empty() )
			{
				for ( auto & member : qualified )
				{
					auto type = getUnqualifiedType( cache, member.type );

					if ( type->getKind() == ast::type::Kind::eArray )
					{
						result->declMember( member.name
							, std::static_pointer_cast< ast::type::Array >( type ) );
					}
					else if ( type->getKind() == ast::type::Kind::eStruct )
					{
						result->declMember( member.name
							, std::static_pointer_cast< ast::type::Struct >( type ) );
					}
					else
					{
						result->declMember( member.name
							, type );
					}
				}
			}

			return result;
		}

		ast::type::ArrayPtr getUnqualifiedType( ast::type::TypesCache & cache
			, ast::type::Array const & qualified )
		{
			return cache.getArray( getUnqualifiedType( cache, qualified.getType() ), qualified.getArraySize() );
		}

		ast::type::SamplerPtr getUnqualifiedType( ast::type::TypesCache & cache
			, ast::type::Sampler const & qualified )
		{
			return cache.getSampler( qualified.isComparison() );
		}

		ast::type::SampledImagePtr getUnqualifiedType( ast::type::TypesCache & cache
			, ast::type::SampledImage const & qualified )
		{
			return cache.getSampledImage( qualified.getConfig() );
		}

		ast::type::ImagePtr getUnqualifiedType( ast::type::TypesCache & cache
			, ast::type::Image const & qualified )
		{
			return cache.getImage( qualified.getConfig() );
		}

		ast::type::TypePtr getUnqualifiedType( ast::type::TypesCache & cache
			, ast::type::Type const & qualified )
		{
			ast::type::TypePtr result;

			if ( qualified.getKind() == ast::type::Kind::eArray )
			{
				result = getUnqualifiedType( cache, static_cast< ast::type::Array const & >( qualified ) );
			}
			else if ( qualified.getKind() == ast::type::Kind::eStruct )
			{
				result = getUnqualifiedType( cache, static_cast< ast::type::Struct const & >( qualified ) );
			}
			else if ( qualified.getKind() == ast::type::Kind::eImage )
			{
				result = getUnqualifiedType( cache, static_cast< ast::type::Image const & >( qualified ) );
			}
			else if ( qualified.getKind() == ast::type::Kind::eSampledImage )
			{
				result = getUnqualifiedType( cache, static_cast< ast::type::SampledImage const & >( qualified ) );
			}
			else if ( qualified.getKind() == ast::type::Kind::eSampler )
			{
				result = getUnqualifiedType( cache, static_cast< ast::type::Sampler const & >( qualified ) );
			}
			else if ( qualified.isMember() )
			{
				result = cache.getBasicType( qualified.getKind() );
			}

			return result;
		}

		ast::type::TypePtr getUnqualifiedType( ast::type::TypesCache & cache
			, ast::type::TypePtr qualified )
		{
			ast::type::TypePtr result = getUnqualifiedType( cache, *qualified );
			return result
				? result
				: qualified;
		}

		ast::type::MemoryLayout getMemoryLayout( ast::type::Type const & type )
		{
			ast::type::MemoryLayout result{ ast::type::MemoryLayout::eStd430 };
			auto kind = type.getKind();

			if ( kind == ast::type::Kind::eArray )
			{
				if ( type.isMember() )
				{
					result = getMemoryLayout( *type.getParent() );
				}
				else
				{
					result = getMemoryLayout( *static_cast< ast::type::Array const & >( type ).getType() );
				}
			}
			else if ( kind == ast::type::Kind::eStruct )
			{
				auto & structType = static_cast< ast::type::Struct const & >( type );
				result = structType.getMemoryLayout();
			}
			else if ( type.isMember() )
			{
				result = getMemoryLayout( *type.getParent() );
			}

			return result;
		}

		void writeArrayStride( Module & module
			, ast::type::TypePtr type
			, uint32_t typeId
			, uint32_t arrayStride )
		{
			auto kind = getNonArrayKind( type );

			if ( kind != ast::type::Kind::eImage
				&& kind != ast::type::Kind::eSampledImage
				&& kind != ast::type::Kind::eSampler )
			{
				module.decorate( typeId
					, IdList
					{
						uint32_t( spv::DecorationArrayStride ),
						arrayStride
					} );
			}
		}

		Instruction::Config const & getConfig( spv::Op opCode )
		{
			static Instruction::Config dummy{};

			switch ( opCode )
			{
			case spv::OpSource:
				return SourceInstruction::config;
			case spv::OpName:
				return NameInstruction::config;
			case spv::OpMemberName:
				return MemberNameInstruction::config;
			case spv::OpExtension:
				return ExtensionInstruction::config;
			case spv::OpExtInstImport:
				return ExtInstImportInstruction::config;
			case spv::OpExtInst:
				return ExtInstInstruction::config;
			case spv::OpMemoryModel:
				return MemoryModelInstruction::config;
			case spv::OpEntryPoint:
				return EntryPointInstruction::config;
			case spv::OpExecutionMode:
				return ExecutionModeInstruction::config;
			case spv::OpCapability:
				return CapabilityInstruction::config;
			case spv::OpTypeVoid:
				return VoidTypeInstruction::config;
			case spv::OpTypeBool:
				return BooleanTypeInstruction::config;
			case spv::OpTypeInt:
				return IntTypeInstruction::config;
			case spv::OpTypeFloat:
				return FloatTypeInstruction::config;
			case spv::OpTypeVector:
				return VectorTypeInstruction::config;
			case spv::OpTypeMatrix:
				return MatrixTypeInstruction::config;
			case spv::OpTypeImage:
				return ImageTypeInstruction::config;
			case spv::OpTypeSampler:
				return SamplerTypeInstruction::config;
			case spv::OpTypeSampledImage:
				return SampledImageTypeInstruction::config;
			case spv::OpTypeArray:
				return ArrayTypeInstruction::config;
			case spv::OpTypeRuntimeArray:
				return RuntimeArrayTypeInstruction::config;
			case spv::OpTypeStruct:
				return StructTypeInstruction::config;
			case spv::OpTypePointer:
				return PointerTypeInstruction::config;
			case spv::OpTypeFunction:
				return FunctionTypeInstruction::config;
			case spv::OpConstantTrue:
				return ConstantTrueInstruction::config;
			case spv::OpConstantFalse:
				return ConstantFalseInstruction::config;
			case spv::OpConstant:
				return ConstantInstruction::config;
			case spv::OpConstantComposite:
				return ConstantCompositeInstruction::config;
			case spv::OpSpecConstantTrue:
				return SpecConstantTrueInstruction::config;
			case spv::OpSpecConstantFalse:
				return SpecConstantFalseInstruction::config;
			case spv::OpSpecConstant:
				return SpecConstantInstruction::config;
			case spv::OpSpecConstantComposite:
				return SpecConstantCompositeInstruction::config;
			case spv::OpSpecConstantOp:
				return SpecConstantOpInstruction::config;
			case spv::OpFunction:
				return FunctionInstruction::config;
			case spv::OpFunctionParameter:
				return FunctionParameterInstruction::config;
			case spv::OpFunctionEnd:
				return FunctionEndInstruction::config;
			case spv::OpFunctionCall:
				return FunctionCallInstruction::config;
			case spv::OpVariable:
				return VariableInstruction::config;
			case spv::OpImageTexelPointer:
				return ImageTexelPointerInstruction::config;
			case spv::OpLoad:
				return LoadInstruction::config;
			case spv::OpStore:
				return StoreInstruction::config;
			case spv::OpAccessChain:
				return AccessChainInstruction::config;
			case spv::OpDecorate:
				return DecorateInstruction::config;
			case spv::OpMemberDecorate:
				return MemberDecorateInstruction::config;
			case spv::OpVectorShuffle:
				return VectorShuffleInstruction::config;
			case spv::OpCompositeConstruct:
				return CompositeConstructInstruction::config;
			case spv::OpCompositeExtract:
				return CompositeExtractInstruction::config;
			case spv::OpTranspose:
				return IntrinsicInstructionT< spv::OpTranspose >::config;
			case spv::OpImageSampleImplicitLod:
				return TextureAccessInstructionT< spv::OpImageSampleImplicitLod >::config;
			case spv::OpImageSampleExplicitLod:
				return TextureAccessInstructionT< spv::OpImageSampleExplicitLod >::config;
			case spv::OpImageSampleDrefImplicitLod:
				return TextureAccessInstructionT< spv::OpImageSampleDrefImplicitLod >::config;
			case spv::OpImageSampleDrefExplicitLod:
				return TextureAccessInstructionT< spv::OpImageSampleDrefExplicitLod >::config;
			case spv::OpImageSampleProjImplicitLod:
				return TextureAccessInstructionT< spv::OpImageSampleProjImplicitLod >::config;
			case spv::OpImageSampleProjExplicitLod:
				return TextureAccessInstructionT< spv::OpImageSampleProjExplicitLod >::config;
			case spv::OpImageSampleProjDrefImplicitLod:
				return TextureAccessInstructionT< spv::OpImageSampleProjDrefImplicitLod >::config;
			case spv::OpImageSampleProjDrefExplicitLod:
				return TextureAccessInstructionT< spv::OpImageSampleProjDrefExplicitLod >::config;
			case spv::OpImageFetch:
				return TextureAccessInstructionT< spv::OpImageFetch >::config;
			case spv::OpImageGather:
				return TextureAccessInstructionT< spv::OpImageGather >::config;
			case spv::OpImageDrefGather:
				return TextureAccessInstructionT< spv::OpImageDrefGather >::config;
			case spv::OpImageRead:
				return ImageAccessInstructionT< spv::OpImageRead >::config;
			case spv::OpImageWrite:
				return ImageAccessInstructionT< spv::OpImageWrite >::config;
			case spv::OpImage:
				return ImageInstruction::config;
			case spv::OpImageQueryFormat:
				return ImageAccessInstructionT< spv::OpImageQueryFormat >::config;
			case spv::OpImageQueryOrder:
				return ImageAccessInstructionT< spv::OpImageQueryOrder >::config;
			case spv::OpImageQuerySizeLod:
				return ImageAccessInstructionT< spv::OpImageQuerySizeLod >::config;
			case spv::OpImageQuerySize:
				return ImageAccessInstructionT< spv::OpImageQuerySize >::config;
			case spv::OpImageQueryLod:
				return ImageAccessInstructionT< spv::OpImageQueryLod >::config;
			case spv::OpImageQueryLevels:
				return ImageAccessInstructionT< spv::OpImageQueryLevels >::config;
			case spv::OpImageQuerySamples:
				return ImageAccessInstructionT< spv::OpImageQuerySamples >::config;
			case spv::OpConvertFToU:
				return UnInstructionT< spv::OpConvertFToU >::config;
			case spv::OpConvertFToS:
				return UnInstructionT< spv::OpConvertFToS >::config;
			case spv::OpConvertSToF:
				return UnInstructionT< spv::OpConvertSToF >::config;
			case spv::OpConvertUToF:
				return UnInstructionT< spv::OpConvertUToF >::config;
			case spv::OpUConvert:
				return UnInstructionT< spv::OpUConvert >::config;
			case spv::OpSConvert:
				return UnInstructionT< spv::OpSConvert >::config;
			case spv::OpFConvert:
				return UnInstructionT< spv::OpFConvert >::config;
			case spv::OpQuantizeToF16:
				return UnInstructionT< spv::OpQuantizeToF16 >::config;
			case spv::OpConvertPtrToU:
				return UnInstructionT< spv::OpConvertPtrToU >::config;
			case spv::OpSatConvertSToU:
				return UnInstructionT< spv::OpSatConvertSToU >::config;
			case spv::OpSatConvertUToS:
				return UnInstructionT< spv::OpSatConvertUToS >::config;
			case spv::OpConvertUToPtr:
				return UnInstructionT< spv::OpConvertUToPtr >::config;
			case spv::OpPtrCastToGeneric:
				return UnInstructionT< spv::OpPtrCastToGeneric >::config;
			case spv::OpGenericCastToPtr:
				return UnInstructionT< spv::OpGenericCastToPtr >::config;
			case spv::OpGenericCastToPtrExplicit:
				return UnInstructionT< spv::OpGenericCastToPtrExplicit >::config;
			case spv::OpBitcast:
				return UnInstructionT< spv::OpBitcast >::config;
			case spv::OpSNegate:
				return UnInstructionT< spv::OpSNegate >::config;
			case spv::OpFNegate:
				return UnInstructionT< spv::OpFNegate >::config;
			case spv::OpIAdd:
				return UnInstructionT < spv::OpIAdd >::config;
			case spv::OpFAdd:
				return UnInstructionT < spv::OpFAdd >::config;
			case spv::OpISub:
				return UnInstructionT < spv::OpISub >::config;
			case spv::OpFSub:
				return UnInstructionT < spv::OpFSub >::config;
			case spv::OpIMul:
				return UnInstructionT < spv::OpIMul >::config;
			case spv::OpFMul:
				return UnInstructionT < spv::OpFMul >::config;
			case spv::OpUDiv:
				return UnInstructionT < spv::OpUDiv >::config;
			case spv::OpSDiv:
				return UnInstructionT < spv::OpSDiv >::config;
			case spv::OpFDiv:
				return UnInstructionT < spv::OpFDiv >::config;
			case spv::OpUMod:
				return UnInstructionT < spv::OpUMod >::config;
			case spv::OpSRem:
				return UnInstructionT < spv::OpSRem >::config;
			case spv::OpSMod:
				return UnInstructionT < spv::OpSMod >::config;
			case spv::OpFRem:
				return UnInstructionT < spv::OpFRem >::config;
			case spv::OpFMod:
				return UnInstructionT < spv::OpFMod >::config;
			case spv::OpVectorTimesScalar:
				return UnInstructionT < spv::OpVectorTimesScalar >::config;
			case spv::OpMatrixTimesScalar:
				return UnInstructionT < spv::OpMatrixTimesScalar >::config;
			case spv::OpVectorTimesMatrix:
				return UnInstructionT < spv::OpVectorTimesMatrix >::config;
			case spv::OpMatrixTimesVector:
				return UnInstructionT < spv::OpMatrixTimesVector >::config;
			case spv::OpMatrixTimesMatrix:
				return UnInstructionT < spv::OpMatrixTimesMatrix >::config;
			case spv::OpOuterProduct:
				return IntrinsicInstructionT< spv::OpOuterProduct >::config;
			case spv::OpDot:
				return IntrinsicInstructionT< spv::OpDot >::config;
			case spv::OpIAddCarry:
				return IntrinsicInstructionT< spv::OpIAddCarry >::config;
			case spv::OpISubBorrow:
				return IntrinsicInstructionT< spv::OpISubBorrow >::config;
			case spv::OpUMulExtended:
				return IntrinsicInstructionT< spv::OpUMulExtended >::config;
			case spv::OpSMulExtended:
				return IntrinsicInstructionT< spv::OpSMulExtended >::config;
			case spv::OpAny:
				return IntrinsicInstructionT< spv::OpAny >::config;
			case spv::OpAll:
				return IntrinsicInstructionT< spv::OpAll >::config;
			case spv::OpIsNan:
				return IntrinsicInstructionT< spv::OpIsNan >::config;
			case spv::OpIsInf:
				return IntrinsicInstructionT< spv::OpIsInf >::config;
			case spv::OpIsFinite:
				return IntrinsicInstructionT< spv::OpIsFinite >::config;
			case spv::OpIsNormal:
				return IntrinsicInstructionT< spv::OpIsNormal >::config;
			case spv::OpSignBitSet:
				return IntrinsicInstructionT< spv::OpSignBitSet >::config;
			case spv::OpLogicalEqual:
				return BinInstructionT < spv::OpLogicalEqual >::config;
			case spv::OpLogicalNotEqual:
				return BinInstructionT < spv::OpLogicalNotEqual >::config;
			case spv::OpLogicalOr:
				return BinInstructionT < spv::OpLogicalOr >::config;
			case spv::OpLogicalAnd:
				return BinInstructionT < spv::OpLogicalAnd >::config;
			case spv::OpLogicalNot:
				return BinInstructionT < spv::OpLogicalNot >::config;
			case spv::OpSelect:
				return SelectInstruction::config;
			case spv::OpIEqual:
				return BinInstructionT < spv::OpIEqual >::config;
			case spv::OpINotEqual:
				return BinInstructionT < spv::OpINotEqual >::config;
			case spv::OpUGreaterThan:
				return BinInstructionT < spv::OpUGreaterThan >::config;
			case spv::OpSGreaterThan:
				return BinInstructionT < spv::OpSGreaterThan >::config;
			case spv::OpUGreaterThanEqual:
				return BinInstructionT < spv::OpUGreaterThanEqual >::config;
			case spv::OpSGreaterThanEqual:
				return BinInstructionT < spv::OpSGreaterThanEqual >::config;
			case spv::OpULessThan:
				return BinInstructionT < spv::OpULessThan >::config;
			case spv::OpSLessThan:
				return BinInstructionT < spv::OpSLessThan >::config;
			case spv::OpULessThanEqual:
				return BinInstructionT < spv::OpULessThanEqual >::config;
			case spv::OpSLessThanEqual:
				return BinInstructionT < spv::OpSLessThanEqual >::config;
			case spv::OpFOrdEqual:
				return BinInstructionT < spv::OpFOrdEqual >::config;
			case spv::OpFUnordEqual:
				return BinInstructionT < spv::OpFUnordEqual >::config;
			case spv::OpFOrdNotEqual:
				return BinInstructionT < spv::OpFOrdNotEqual >::config;
			case spv::OpFUnordNotEqual:
				return BinInstructionT < spv::OpFUnordNotEqual >::config;
			case spv::OpFOrdLessThan:
				return BinInstructionT < spv::OpFOrdLessThan >::config;
			case spv::OpFUnordLessThan:
				return BinInstructionT < spv::OpFUnordLessThan >::config;
			case spv::OpFOrdGreaterThan:
				return BinInstructionT < spv::OpFOrdGreaterThan >::config;
			case spv::OpFUnordGreaterThan:
				return BinInstructionT < spv::OpFUnordGreaterThan >::config;
			case spv::OpFOrdLessThanEqual:
				return BinInstructionT < spv::OpFOrdLessThanEqual >::config;
			case spv::OpFUnordLessThanEqual:
				return BinInstructionT < spv::OpFUnordLessThanEqual >::config;
			case spv::OpFOrdGreaterThanEqual:
				return BinInstructionT < spv::OpFOrdGreaterThanEqual >::config;
			case spv::OpFUnordGreaterThanEqual:
				return BinInstructionT < spv::OpFUnordGreaterThanEqual >::config;
			case spv::OpShiftRightLogical:
				return BinInstructionT < spv::OpShiftRightLogical >::config;
			case spv::OpShiftRightArithmetic:
				return BinInstructionT < spv::OpShiftRightArithmetic >::config;
			case spv::OpShiftLeftLogical:
				return BinInstructionT < spv::OpShiftLeftLogical >::config;
			case spv::OpBitwiseOr:
				return BinInstructionT < spv::OpBitwiseOr >::config;
			case spv::OpBitwiseXor:
				return BinInstructionT < spv::OpBitwiseXor >::config;
			case spv::OpBitwiseAnd:
				return BinInstructionT < spv::OpBitwiseAnd >::config;
			case spv::OpNot:
				return UnInstructionT < spv::OpNot >::config;
			case spv::OpBitFieldInsert:
				return IntrinsicInstructionT< spv::OpBitFieldInsert >::config;
			case spv::OpBitFieldSExtract:
				return IntrinsicInstructionT< spv::OpBitFieldSExtract >::config;
			case spv::OpBitFieldUExtract:
				return IntrinsicInstructionT< spv::OpBitFieldUExtract >::config;
			case spv::OpBitReverse:
				return IntrinsicInstructionT< spv::OpBitReverse >::config;
			case spv::OpBitCount:
				return IntrinsicInstructionT< spv::OpBitCount >::config;
			case spv::OpDPdx:
				return IntrinsicInstructionT< spv::OpDPdx >::config;
			case spv::OpDPdy:
				return IntrinsicInstructionT< spv::OpDPdy >::config;
			case spv::OpFwidth:
				return IntrinsicInstructionT< spv::OpFwidth >::config;
			case spv::OpDPdxFine:
				return IntrinsicInstructionT< spv::OpDPdxFine >::config;
			case spv::OpDPdyFine:
				return IntrinsicInstructionT< spv::OpDPdyFine >::config;
			case spv::OpFwidthFine:
				return IntrinsicInstructionT< spv::OpFwidthFine >::config;
			case spv::OpDPdxCoarse:
				return IntrinsicInstructionT< spv::OpDPdxCoarse >::config;
			case spv::OpDPdyCoarse:
				return IntrinsicInstructionT< spv::OpDPdyCoarse >::config;
			case spv::OpFwidthCoarse:
				return IntrinsicInstructionT< spv::OpFwidthCoarse >::config;
			case spv::OpEmitVertex:
				return IntrinsicInstructionT< spv::OpEmitVertex >::config;
			case spv::OpEndPrimitive:
				return IntrinsicInstructionT< spv::OpEndPrimitive >::config;
			case spv::OpEmitStreamVertex:
				return IntrinsicInstructionT< spv::OpEmitStreamVertex >::config;
			case spv::OpEndStreamPrimitive:
				return IntrinsicInstructionT< spv::OpEndStreamPrimitive >::config;
			case spv::OpControlBarrier:
				return IntrinsicInstructionT< spv::OpControlBarrier >::config;
			case spv::OpMemoryBarrier:
				return IntrinsicInstructionT< spv::OpMemoryBarrier >::config;
			case spv::OpAtomicLoad:
				return IntrinsicInstructionT< spv::OpAtomicLoad >::config;
			case spv::OpAtomicStore:
				return IntrinsicInstructionT< spv::OpAtomicStore >::config;
			case spv::OpAtomicExchange:
				return IntrinsicInstructionT< spv::OpAtomicExchange >::config;
			case spv::OpAtomicCompareExchange:
				return IntrinsicInstructionT< spv::OpAtomicCompareExchange >::config;
			case spv::OpAtomicCompareExchangeWeak:
				return IntrinsicInstructionT< spv::OpAtomicCompareExchangeWeak >::config;
			case spv::OpAtomicIIncrement:
				return IntrinsicInstructionT< spv::OpAtomicIIncrement >::config;
			case spv::OpAtomicIDecrement:
				return IntrinsicInstructionT< spv::OpAtomicIDecrement >::config;
			case spv::OpAtomicIAdd:
				return IntrinsicInstructionT< spv::OpAtomicIAdd >::config;
			case spv::OpAtomicISub:
				return IntrinsicInstructionT< spv::OpAtomicISub >::config;
			case spv::OpAtomicSMin:
				return IntrinsicInstructionT< spv::OpAtomicSMin >::config;
			case spv::OpAtomicUMin:
				return IntrinsicInstructionT< spv::OpAtomicUMin >::config;
			case spv::OpAtomicSMax:
				return IntrinsicInstructionT< spv::OpAtomicSMax >::config;
			case spv::OpAtomicUMax:
				return IntrinsicInstructionT< spv::OpAtomicUMax >::config;
			case spv::OpAtomicAnd:
				return IntrinsicInstructionT< spv::OpAtomicAnd >::config;
			case spv::OpAtomicOr:
				return IntrinsicInstructionT< spv::OpAtomicOr >::config;
			case spv::OpAtomicXor:
				return IntrinsicInstructionT< spv::OpAtomicXor >::config;
			case spv::OpLoopMerge:
				return LoopMergeInstruction::config;
			case spv::OpSelectionMerge:
				return SelectionMergeInstruction::config;
			case spv::OpLabel:
				return LabelInstruction::config;
			case spv::OpBranch:
				return BranchInstruction::config;
			case spv::OpBranchConditional:
				return BranchConditionalInstruction::config;
			case spv::OpSwitch:
				return SwitchInstruction::config;
			case spv::OpKill:
				return KillInstruction::config;
			case spv::OpReturn:
				return ReturnInstruction::config;
			case spv::OpReturnValue:
				return ReturnValueInstruction::config;
			case spv::OpImageSparseSampleImplicitLod:
				return ImageAccessInstructionT< spv::OpImageSparseSampleImplicitLod >::config;
			case spv::OpImageSparseSampleExplicitLod:
				return ImageAccessInstructionT< spv::OpImageSparseSampleExplicitLod >::config;
			case spv::OpImageSparseSampleDrefImplicitLod:
				return ImageAccessInstructionT< spv::OpImageSparseSampleDrefImplicitLod >::config;
			case spv::OpImageSparseSampleDrefExplicitLod:
				return ImageAccessInstructionT< spv::OpImageSparseSampleDrefExplicitLod >::config;
			case spv::OpImageSparseSampleProjImplicitLod:
				return ImageAccessInstructionT< spv::OpImageSparseSampleProjImplicitLod >::config;
			case spv::OpImageSparseSampleProjExplicitLod:
				return ImageAccessInstructionT< spv::OpImageSparseSampleProjExplicitLod >::config;
			case spv::OpImageSparseSampleProjDrefImplicitLod:
				return ImageAccessInstructionT< spv::OpImageSparseSampleProjDrefImplicitLod >::config;
			case spv::OpImageSparseSampleProjDrefExplicitLod:
				return ImageAccessInstructionT< spv::OpImageSparseSampleProjDrefExplicitLod >::config;
			case spv::OpImageSparseFetch:
				return ImageAccessInstructionT< spv::OpImageSparseFetch >::config;
			case spv::OpImageSparseGather:
				return ImageAccessInstructionT< spv::OpImageSparseGather >::config;
			case spv::OpImageSparseDrefGather:
				return ImageAccessInstructionT< spv::OpImageSparseDrefGather >::config;
			case spv::OpImageSparseTexelsResident:
				return ImageAccessInstructionT< spv::OpImageSparseTexelsResident >::config;
			case spv::OpImageSparseRead:
				return ImageAccessInstructionT < spv::OpImageSparseRead >::config;
			default:
				assert( false && "Unsupported Instruction operator" );
				return dummy;
			}
		}
	}

	//*************************************************************************

	size_t IdListHasher::operator()( IdList const & list )const
	{
		assert( !list.empty() );
		auto hash = std::hash< spv::Id >{}( list[0] );

		std::for_each( list.begin() + 1u
			, list.end()
			, [&hash]( spv::Id id )
			{
				ast::type::hashCombine( hash, id );
			} );

		return hash;
	}

	//*************************************************************************

	Instruction::Instruction( Config const & config
		, spv::Op op
		, std::optional< spv::Id > returnTypeId
		, std::optional< spv::Id > resultId
		, IdList operands
		, std::optional< std::string > name
		, std::optional< std::map< int32_t, spv::Id > > labels )
		: returnTypeId{ returnTypeId }
		, resultId{ resultId }
		, operands{ operands }
		, packedName{ std::nullopt }
		, config{ config }
		, name{ name }
		, labels{ labels }
	{
		if ( bool( this->name ) )
		{
			packedName = packString( this->name.value() );
		}

		this->op.op = op;
		this->op.opCount = uint16_t( 1u
			+ ( bool( this->returnTypeId ) ? 1u : 0u )
			+ ( bool( this->resultId ) ? 1u : 0u )
			+ this->operands.size()
			+ ( bool( this->packedName ) ? this->packedName.value().size() : 0u ) );

		assertType( *this, config );
	}

	Instruction::Instruction( Config const & config
		, Op op
		, UInt32ListCIt & buffer )
		: op{ op }
		, config{ config }
	{
		uint32_t index = 1u;
		auto popValue = [&buffer, &index]()
		{
			auto result = *buffer;
			++buffer;
			++index;
			return result;
		};

		if ( config.hasReturnTypeId )
		{
			returnTypeId = popValue();
		}

		if ( config.hasResultId )
		{
			resultId = popValue();
		}

		if ( config.hasName )
		{
			packedName = deserializePackedName( buffer, index );
			name = unpackString( packedName.value() );
		}

		if ( config.operandsCount )
		{
			auto count = op.opCount - index;
			operands.resize( count );

			for ( auto & operand : operands )
			{
				operand = popValue();
			}
		}
		else if ( config.hasLabels )
		{
			auto count = ( op.opCount - index ) / 2u;
			labels = std::map< int32_t, spv::Id >{};

			for ( auto i = 0u; i < count; ++i )
			{
				auto label = popValue();
				labels.value()[label] = popValue();
			}
		}
	}

	Instruction::~Instruction()
	{
	}

	void Instruction::serialize( UInt32List & buffer
		, Instruction const & instruction )
	{
		assertType( instruction, instruction.config );

		auto pushValue = [&buffer]( uint32_t value )
		{
			buffer.push_back( value );
		};

		pushValue( instruction.op.opValue );

		if ( instruction.returnTypeId )
		{
			pushValue( instruction.returnTypeId.value() );
		}
		
		if ( instruction.resultId )
		{
			pushValue( instruction.resultId.value() );
		}

		if ( instruction.packedName )
		{
			for ( auto & c : instruction.packedName.value() )
			{
				pushValue( c );
			}
		}

		if ( !instruction.operands.empty() )
		{
			for ( auto & operand : instruction.operands )
			{
				pushValue( operand );
			}
		}

		if ( instruction.labels )
		{
			for ( auto & label : instruction.labels.value() )
			{
				pushValue( label.first );
				pushValue( label.second );
			}
		}
	}

	InstructionPtr Instruction::deserialize( UInt32ListCIt & buffer )
	{
		auto index = 0u;
		auto popValue = [&buffer, &index]()
		{
			auto result = *buffer;
			++buffer;
			++index;
			return result;
		};
		spirv::Op op;
		op.opValue = popValue();
		assert( op.opCode != spv::OpNop );
		auto & config = getConfig( spv::Op( op.opCode ) );
		return std::make_unique< Instruction >( config, op, buffer );
	}

	//*************************************************************************

	Block Block::deserialize( InstructionPtr firstInstruction
		, InstructionListIt & buffer
		, InstructionListIt & end )
	{
		auto popValue = [&buffer]()
		{
			auto result = std::move( *buffer );
			++buffer;
			return result;
		};
		auto isLastBlockInstruction = []( spv::Op opCode )
		{
			return opCode == spv::OpBranch
				|| opCode == spv::OpBranchConditional
				|| opCode == spv::OpFunctionEnd;
		};

		spv::Op op = spv::OpNop;
		Block result;
		result.label = firstInstruction->resultId.value();
		result.instructions.emplace_back( std::move( firstInstruction ) );

		while ( buffer != end
			&& !isLastBlockInstruction( op ) )
		{
			op = spv::Op( ( *buffer )->op.opCode );

			if ( !isLastBlockInstruction( op ) )
			{
				result.instructions.emplace_back( popValue() );
			}
		}

		if ( buffer != end )
		{
			result.blockEnd = std::move( popValue() );
		}

		return result;
	}

	//*************************************************************************

	Function Function::deserialize( InstructionListIt & buffer
		, InstructionListIt & end )
	{
		auto popValue = [&buffer]()
		{
			auto result = std::move( *buffer );
			++buffer;
			return result;
		};
		auto isDeclarationInstruction = []( spv::Op opCode )
		{
			return opCode == spv::OpFunction
				|| opCode == spv::OpFunctionParameter
				|| opCode == spv::OpVariable;
		};

		Function result;

		while ( buffer != end )
		{
			auto instruction = popValue();

			if ( isDeclarationInstruction( spv::Op( instruction->op.opCode ) ) )
			{
				result.declaration.emplace_back( std::move( instruction ) );
			}
			else
			{
				result.cfg.blocks.emplace_back( Block::deserialize( std::move( instruction ), buffer, end ) );
			}
		}
		
		return result;
	}

	//*************************************************************************

	Module::Module( ast::type::TypesCache & cache
		, spv::MemoryModel memoryModel
		, spv::ExecutionModel executionModel )
		: variables{ &globalDeclarations }
		, memoryModel{ makeInstruction< MemoryModelInstruction >( spv::Id( spv::AddressingModelLogical ), spv::Id( memoryModel ) ) }
		, m_cache{ &cache }
		, m_model{ executionModel }
		, m_currentScopeVariables{ &m_registeredVariables }
	{
		header.push_back( spv::MagicNumber );
		header.push_back( spv::Version );
		header.push_back( 0x00100001 );
		header.push_back( 1u ); // Bound IDs.
		header.push_back( 0u ); // Schema.
		m_currentId = &header[3];
		auto id = getIntermediateResult();
		extensions.push_back( makeInstruction< ExtInstImportInstruction >( id
			, "GLSL.std.450" ) );
		debug.push_back( makeInstruction< SourceInstruction >( spv::Id( spv::SourceLanguageGLSL ), 450u ) );

		switch ( m_model )
		{
		case spv::ExecutionModelVertex:
			capabilities.push_back( makeInstruction< CapabilityInstruction >( spv::Id( spv::CapabilityShader ) ) );
			break;
		case spv::ExecutionModelTessellationControl:
			capabilities.push_back( makeInstruction< CapabilityInstruction >( spv::Id( spv::CapabilityShader ) ) );
			break;
		case spv::ExecutionModelTessellationEvaluation:
			capabilities.push_back( makeInstruction< CapabilityInstruction >( spv::Id( spv::CapabilityShader ) ) );
			break;
		case spv::ExecutionModelGeometry:
			capabilities.push_back( makeInstruction< CapabilityInstruction >( spv::Id( spv::CapabilityShader ) ) );
			break;
		case spv::ExecutionModelFragment:
			capabilities.push_back( makeInstruction< CapabilityInstruction >( spv::Id( spv::CapabilityShader ) ) );
			break;
		case spv::ExecutionModelGLCompute:
			capabilities.push_back( makeInstruction< CapabilityInstruction >( spv::Id( spv::CapabilityShader ) ) );
			break;
		case spv::ExecutionModelKernel:
			capabilities.push_back( makeInstruction< CapabilityInstruction >( spv::Id( spv::CapabilityKernel ) ) );
			break;
		default:
			assert( false && "Unsupported ExecutionModel" );
			break;
		}
	}

	Module::Module( Header const & header
		, InstructionList && instructions )
		: variables{ &globalDeclarations }
	{
		this->header.push_back( header.magic );
		this->header.push_back( header.version );
		this->header.push_back( header.builder );
		this->header.push_back( header.boundIds );
		this->header.push_back( header.schema );
		auto it = instructions.begin();

		while ( it != instructions.end() )
		{
			InstructionList * list{ nullptr };

			switch ( spv::Op( ( *it )->op.opCode ) )
			{
			case spv::OpSource:
			case spv::OpName:
			case spv::OpMemberName:
				list = &debug;
				break;
			case spv::OpExtInstImport:
			case spv::OpExtension:
				list = &extensions;
				break;
			case spv::OpCapability:
				list = &capabilities;
				break;
			case spv::OpExecutionMode:
				list = &executionModes;
				break;
			case spv::OpDecorate:
			case spv::OpMemberDecorate:
				list = &decorations;
				break;
			case spv::OpTypeVoid:
			case spv::OpTypeBool:
			case spv::OpTypeInt:
			case spv::OpTypeFloat:
			case spv::OpTypeVector:
			case spv::OpTypeMatrix:
			case spv::OpTypeImage:
			case spv::OpTypeSampler:
			case spv::OpTypeSampledImage:
			case spv::OpTypeArray:
			case spv::OpTypeRuntimeArray:
			case spv::OpTypeStruct:
			case spv::OpTypeOpaque:
			case spv::OpTypePointer:
			case spv::OpTypeFunction:
			case spv::OpTypeEvent:
			case spv::OpTypeDeviceEvent:
			case spv::OpTypeReserveId:
			case spv::OpTypeQueue:
			case spv::OpTypePipe:
			case spv::OpTypeForwardPointer:
			case spv::OpVariable:
			case spv::OpConstant:
			case spv::OpConstantComposite:
			case spv::OpConstantFalse:
			case spv::OpConstantTrue:
			case spv::OpSpecConstant:
			case spv::OpSpecConstantComposite:
			case spv::OpSpecConstantFalse:
			case spv::OpSpecConstantTrue:
				list = &globalDeclarations;
				break;
			case spv::OpFunction:
				functions.emplace_back( Function::deserialize( it, instructions.end() ) );
				break;
			case spv::OpMemoryModel:
				memoryModel = std::move( *it );
				++it;
				break;
			case spv::OpEntryPoint:
				entryPoint = std::move( *it );
				++it;
				break;
			}

			if ( list )
			{
				list->emplace_back( std::move( *it ) );
				++it;
			}
		}
	}

	Module Module::deserialize( UInt32List const & spirv )
	{
		auto it = spirv.cbegin();
		auto popValue = [&it]( uint32_t & value )
		{
			value = *it;
			++it;
		};
		auto popInstruction = [&it]()
		{
			return spirv::Instruction::deserialize( it );
		};
		Header header{};
		popValue( header.magic );
		popValue( header.version );
		popValue( header.builder );
		popValue( header.boundIds );
		popValue( header.schema );
		assert( header.magic = spv::MagicNumber );
		spirv::InstructionList instructions;

		while ( it != spirv.end() )
		{
			instructions.emplace_back( popInstruction() );
		}

		return Module{ header, std::move( instructions ) };
	}

	spv::Id Module::registerType( ast::type::TypePtr type )
	{
		return registerType( type
			, ast::type::NotMember
			, 0u
			, 0u );
	}

	spv::Id Module::registerPointerType( spv::Id type, spv::StorageClass storage )
	{
		uint64_t key = ( uint64_t( type ) << 32 ) | uint64_t( storage );
		auto it = m_registeredPointerTypes.find( key );

		if ( it == m_registeredPointerTypes.end() )
		{
			spv::Id id{ getNextId() };
			it = m_registeredPointerTypes.emplace( key, id ).first;
			globalDeclarations.push_back( makeInstruction< PointerTypeInstruction >( id
				, spv::Id( storage ), type ) );
		}

		return it->second;
	}

	void Module::decorate( spv::Id id, spv::Decoration decoration )
	{
		decorate( id, IdList{ spv::Id( decoration ) } );
	}

	void Module::decorate( spv::Id id
		, IdList const & decoration )
	{
		IdList operands;
		operands.push_back( id );
		operands.insert( operands.end(), decoration.begin(), decoration.end() );
		decorations.push_back( makeInstruction< DecorateInstruction >( operands ) );
	}

	void Module::decorateMember( spv::Id id
		, uint32_t index
		, spv::Decoration decoration )
	{
		decorateMember( id, index, IdList{ spv::Id( decoration ) } );
	}

	void Module::decorateMember( spv::Id id
		, uint32_t index
		, IdList const & decoration )
	{
		IdList operands;
		operands.push_back( id );
		operands.push_back( index );
		operands.insert( operands.end(), decoration.begin(), decoration.end() );
		decorations.push_back( makeInstruction< MemberDecorateInstruction >( operands ) );
	}

	spv::Id Module::loadVariable( spv::Id variable
		, ast::type::TypePtr type
		, Block & currentBlock )
	{
		auto result = getIntermediateResult();
		currentBlock.instructions.push_back( makeInstruction< LoadInstruction >( registerType( type )
			, result
			, variable ) );
		lnkIntermediateResult( result, variable );
		return result;
	}

	void Module::bindVariable( spv::Id varId
		, uint32_t bindingPoint
		, uint32_t descriptorSet )
	{
		decorate( varId, { spv::Id( spv::DecorationBinding ), bindingPoint } );
		decorate( varId, { spv::Id( spv::DecorationDescriptorSet ), descriptorSet } );
	}

	void Module::bindBufferVariable( std::string const & name
		, uint32_t bindingPoint
		, uint32_t descriptorSet
		, spv::Decoration structDecoration )
	{
		auto varIt = m_currentScopeVariables->find( name );

		if ( varIt != m_currentScopeVariables->end() )
		{
			auto varId = varIt->second;
			decorate( varId, { spv::Id( spv::DecorationBinding ), bindingPoint } );
			decorate( varId, { spv::Id( spv::DecorationDescriptorSet ), descriptorSet } );

			auto typeIt = m_registeredVariablesTypes.find( varId );

			if ( typeIt != m_registeredVariablesTypes.end() )
			{
				auto typeId = typeIt->second;
				decorate( typeId, structDecoration );
			}
		}
	}

	VariableInfo & Module::registerVariable( std::string const & name
		, spv::StorageClass storage
		, ast::type::TypePtr type
		, VariableInfo & info )
	{
		auto it = m_currentScopeVariables->find( name );

		if ( it == m_currentScopeVariables->end() )
		{
			spv::Id id{ getNextId() };

			if ( type->getKind() != ast::type::Kind::eStruct
				|| std::static_pointer_cast< ast::type::Struct >( type )->getName() != name )
			{
				debug.push_back( makeInstruction< NameInstruction >( id, name ) );
			}
			else if ( type->getKind() == ast::type::Kind::eStruct
				|| std::static_pointer_cast< ast::type::Struct >( type )->getName() == name )
			{
				debug.push_back( makeInstruction< NameInstruction >( id, name + "Inst" ) );
			}

			auto builtin = getBuiltin( name );

			if ( builtin != spv::BuiltIn( -1 ) )
			{
				decorate( id, { spv::Id( spv::DecorationBuiltIn ), spv::Id( builtin ) } );
			}

			auto rawTypeId = registerType( type );
			auto varTypeId = registerPointerType( rawTypeId, storage );

			if ( storage == spv::StorageClassFunction
				&& m_currentFunction )
			{
				it = m_currentScopeVariables->emplace( name, id ).first;
				m_currentFunction->variables.push_back( makeInstruction< VariableInstruction >( varTypeId
					, id
					, spv::Id( storage ) ) );
			}
			else
			{
				it = m_registeredVariables.emplace( name, id ).first;
				globalDeclarations.push_back( makeInstruction< VariableInstruction >( varTypeId
					, id
					, spv::Id( storage ) ) );
			}

			m_registeredVariablesTypes.emplace( id, rawTypeId );
		}

		info.id = it->second;
		return info;
	}

	spv::Id Module::registerSpecConstant( std::string name
		, uint32_t location
		, ast::type::TypePtr type
		, ast::expr::Literal const & value )
	{
		auto it = m_currentScopeVariables->find( name );

		if ( it == m_currentScopeVariables->end() )
		{
			spv::Id id{ getNextId() };
			it = m_currentScopeVariables->emplace( name, id ).first;
			auto rawTypeId = registerType( type );
			IdList operands;
			debug.push_back( makeInstruction< NameInstruction >( id, name ) );

			if ( value.getLiteralType() == ast::expr::LiteralType::eBool )
			{
				if ( value.getValue< ast::expr::LiteralType::eBool >() )
				{
					globalDeclarations.emplace_back( makeInstruction< SpecConstantTrueInstruction >( rawTypeId, id ) );
				}
				else
				{
					globalDeclarations.emplace_back( makeInstruction< SpecConstantFalseInstruction >( rawTypeId, id ) );
				}
			}
			else
			{
				switch ( value.getLiteralType() )
				{
				case ast::expr::LiteralType::eInt:
					operands.emplace_back( uint32_t( value.getValue< ast::expr::LiteralType::eInt >() ) );
					break;
				case ast::expr::LiteralType::eUInt:
					operands.emplace_back( value.getValue< ast::expr::LiteralType::eUInt >() );
					break;
				case ast::expr::LiteralType::eFloat:
				{
					operands.resize( sizeof( float ) / sizeof( uint32_t ) );
					auto dst = reinterpret_cast< float * >( operands.data() );
					*dst = value.getValue< ast::expr::LiteralType::eFloat >();
				}
				break;
				case ast::expr::LiteralType::eDouble:
				{
					operands.resize( sizeof( double ) / sizeof( uint32_t ) );
					auto dst = reinterpret_cast< double * >( operands.data() );
					*dst = value.getValue< ast::expr::LiteralType::eDouble >();
				}
				break;
				}

				globalDeclarations.emplace_back( makeInstruction< SpecConstantInstruction >( rawTypeId
					, id
					, operands ) );
			}

			decorate( id, { spv::Id( spv::DecorationSpecId ), spv::Id( location ) } );
			m_registeredVariablesTypes.emplace( id, rawTypeId );
			m_registeredConstants.emplace( id, value.getType() );
		}

		return it->second;
	}

	spv::Id Module::registerParameter( ast::type::TypePtr type )
	{
		auto typeId = registerType( type );
		auto paramId = getNextId();
		return paramId;
	}

	spv::Id Module::registerMemberVariableIndex( ast::type::TypePtr type )
	{
		assert( type->isMember() );
		return registerLiteral( type->getIndex() );
	}

	spv::Id Module::registerMemberVariable( spv::Id outer
		, std::string name
		, ast::type::TypePtr type )
	{
		auto it = std::find_if( m_currentScopeVariables->begin()
			, m_currentScopeVariables->end()
			, [outer]( std::map< std::string, spv::Id >::value_type const & pair )
			{
				return pair.second == outer;
			} );
		assert( it != m_currentScopeVariables->end() );
		assert( type->isMember() );
		auto fullName = it->first + "::" + name;
		auto outerId = it->second;
		it = m_currentScopeVariables->find( fullName );

		if ( it == m_currentScopeVariables->end() )
		{
			spv::Id id{ getNextId() };
			m_registeredMemberVariables.insert( { fullName, { outer, id } } );
			it = m_currentScopeVariables->emplace( fullName, id ).first;
			registerLiteral( type->getIndex() );
		}

		return it->second;
	}

	ast::type::Kind Module::getLiteralType( spv::Id litId )const
	{
		auto it = m_registeredConstants.find( litId );
		assert( it != m_registeredConstants.end() );
		return it->second->getKind();
	}

	spv::Id Module::getOuterVariable( spv::Id mbrId )const
	{
		auto itInner = std::find_if( m_registeredMemberVariables.begin()
			, m_registeredMemberVariables.end()
			, [mbrId]( std::map< std::string, std::pair< spv::Id, spv::Id > >::value_type const & pair )
			{
				return pair.second.second == mbrId;
			} );
		assert( itInner != m_registeredMemberVariables.end() );
		
		auto result = itInner->second.first;
		auto itOuter = m_registeredMemberVariables.end();

		while ( ( itOuter = std::find_if( m_registeredMemberVariables.begin()
				, m_registeredMemberVariables.end()
				, [result]( std::map< std::string, std::pair< spv::Id, spv::Id > >::value_type const & pair )
				{
					return pair.second.second == result;
				} ) ) != m_registeredMemberVariables.end() )
		{
			result = itOuter->second.first;
		}

		auto itOutermost = std::find_if( m_currentScopeVariables->begin()
			, m_currentScopeVariables->end()
			, [result]( std::map< std::string, spv::Id >::value_type const & pair )
			{
					return pair.second == result;
			} );
		assert( itOutermost != m_currentScopeVariables->end() );
		result = itOutermost->second;
		return result;
	}

	spv::Id Module::registerLiteral( bool value )
	{
		auto it = m_registeredBoolConstants.find( value );

		if ( it == m_registeredBoolConstants.end() )
		{
			spv::Id result{ getNextId() };
			auto type = registerType( m_cache->getBool() );

			if ( value )
			{
				globalDeclarations.push_back( makeInstruction< ConstantTrueInstruction >( type, result ) );
			}
			else
			{
				globalDeclarations.push_back( makeInstruction< ConstantFalseInstruction >( type, result ) );
			}

			it = m_registeredBoolConstants.emplace( value, result ).first;
			m_registeredConstants.emplace( result, m_cache->getBool() );
		}

		return it->second;
	}

	spv::Id Module::registerLiteral( int32_t value )
	{
		auto it = m_registeredIntConstants.find( value );

		if ( it == m_registeredIntConstants.end() )
		{
			spv::Id result{ getNextId() };
			auto type = registerType( m_cache->getInt() );
			globalDeclarations.push_back( makeInstruction< ConstantInstruction >( type
				, result
				, IdList{ uint32_t( value ) } ) );
			it = m_registeredIntConstants.emplace( value, result ).first;
			m_registeredConstants.emplace( result, m_cache->getInt() );
		}

		return it->second;
	}

	spv::Id Module::registerLiteral( uint32_t value )
	{
		auto it = m_registeredUIntConstants.find( value );

		if ( it == m_registeredUIntConstants.end() )
		{
			spv::Id result{ getNextId() };
			auto type = registerType( m_cache->getUInt() );
			globalDeclarations.push_back( makeInstruction< ConstantInstruction >( type
				, result
				, IdList{ value } ) );
			it = m_registeredUIntConstants.emplace( value, result ).first;
			m_registeredConstants.emplace( result, m_cache->getUInt() );
		}

		return it->second;
	}

	spv::Id Module::registerLiteral( float value )
	{
		auto it = m_registeredFloatConstants.find( value );

		if ( it == m_registeredFloatConstants.end() )
		{
			spv::Id result{ getNextId() };
			auto type = registerType( m_cache->getFloat() );
			globalDeclarations.push_back( makeInstruction< ConstantInstruction >( type
				, result
				, IdList{ *reinterpret_cast< uint32_t * >( &value ) } ) );
			it = m_registeredFloatConstants.emplace( value, result ).first;
			m_registeredConstants.emplace( result, m_cache->getFloat() );
		}

		return it->second;
	}

	spv::Id Module::registerLiteral( double value )
	{
		auto it = m_registeredDoubleConstants.find( value );

		if ( it == m_registeredDoubleConstants.end() )
		{
			spv::Id result{ getNextId() };
			auto type = registerType( m_cache->getDouble() );
			IdList list;
			list.resize( 2u );
			auto dst = reinterpret_cast< double * >( list.data() );
			*dst = value;
			globalDeclarations.push_back( makeInstruction< ConstantInstruction >( type
				, result
				, list ) );
			it = m_registeredDoubleConstants.emplace( value, result ).first;
			m_registeredConstants.emplace( result, m_cache->getDouble() );
		}

		return it->second;
	}

	spv::Id Module::registerLiteral( IdList const & initialisers
		, ast::type::TypePtr type )
	{
		auto typeId = registerType( type );
		auto it = std::find_if( m_registeredCompositeConstants.begin()
			, m_registeredCompositeConstants.end()
			, [initialisers]( std::pair< IdList, spv::Id > const & lookup )
			{
				return lookup.first == initialisers;
			} );

		if ( it == m_registeredCompositeConstants.end() )
		{
			spv::Id result{ getNextId() };
			globalDeclarations.push_back( makeInstruction< ConstantCompositeInstruction >( typeId
				, result
				, initialisers ) );
			m_registeredCompositeConstants.emplace_back( initialisers, result );
			it = m_registeredCompositeConstants.begin() + m_registeredCompositeConstants.size() - 1u;
			m_registeredConstants.emplace( result, type );
		}

		return it->second;
	}

	void Module::registerExtension( std::string const & name )
	{
		extensions.push_back( makeInstruction< ExtensionInstruction >( name ) );
	}

	void Module::registerEntryPoint( spv::Id functionId
		, std::string const & name
		, IdList const & inputs
		, IdList const & outputs )
	{
		IdList operands;
		operands.insert( operands.end(), inputs.begin(), inputs.end() );
		operands.insert( operands.end(), outputs.begin(), outputs.end() );
		entryPoint = makeInstruction< EntryPointInstruction >( spv::Id( m_model )
			, functionId
			, operands
			, name );

		switch ( m_model )
		{
		case spv::ExecutionModelVertex:
			break;
		case spv::ExecutionModelTessellationControl:
			break;
		case spv::ExecutionModelTessellationEvaluation:
			break;
		case spv::ExecutionModelGeometry:
			break;
		case spv::ExecutionModelFragment:
			registerExecutionMode( spv::ExecutionModeOriginLowerLeft );
			break;
		case spv::ExecutionModelGLCompute:
			break;
		case spv::ExecutionModelKernel:
			break;
		case spv::ExecutionModelMax:
			break;
		default:
			break;
		}

		for ( auto & executionMode : m_pendingExecutionModes )
		{
			executionModes.emplace_back( std::move( executionMode ) );
			executionModes.back()->operands[0] = functionId;
		}

		m_pendingExecutionModes.clear();
	}

	void Module::registerExecutionMode( spv::ExecutionMode mode )
	{
		registerExecutionMode( mode, {} );
	}

	void Module::registerExecutionMode( spv::ExecutionMode mode, IdList const & operands )
	{
		if ( !entryPoint || entryPoint->operands.empty() )
		{
			IdList ops;
			ops.push_back( spv::Id( 0u ) );
			ops.push_back( spv::Id( mode ) );
			ops.insert( ops.end(), operands.begin(), operands.end() );
			m_pendingExecutionModes.push_back( makeInstruction< ExecutionModeInstruction >( ops ) );
		}
		else
		{
			IdList ops;
			ops.push_back( entryPoint->resultId.value() );
			ops.push_back( spv::Id( mode ) );
			ops.insert( ops.end(), operands.begin(), operands.end() );
			executionModes.push_back( makeInstruction< ExecutionModeInstruction >( ops ) );
		}
	}

	spv::Id Module::getIntermediateResult()
	{
		spv::Id result{};

		if ( m_freeIntermediates.empty() )
		{
			result = getNextId();
			m_intermediates.insert( result );
			m_freeIntermediates.insert( result );
		}

		result = *m_freeIntermediates.begin();
		m_freeIntermediates.erase( m_freeIntermediates.begin() );
		return result;
	}

	void Module::lnkIntermediateResult( spv::Id intermediate, spv::Id var )
	{
		if ( m_intermediates.end() != m_intermediates.find( intermediate ) )
		{
			m_busyIntermediates.emplace( intermediate, var );
		}
	}

	void Module::putIntermediateResult( spv::Id id )
	{
		//if ( m_intermediates.end() != m_intermediates.find( id ) )
		//{
		//	m_freeIntermediates.insert( id );
		//	auto it = m_busyIntermediates.begin();

		//	while ( it != m_busyIntermediates.end() )
		//	{
		//		if ( it->first == id
		//			|| it->second == id )
		//		{
		//			it = m_busyIntermediates.erase( it );
		//		}
		//		else
		//		{
		//			++it;
		//		}
		//	}
		//}
	}

	spv::Id Module::getNonIntermediate( spv::Id id )
	{
		while ( m_intermediates.end() != m_intermediates.find( id ) )
		{
			id = m_busyIntermediates.find( id )->second;
		}

		return id;
	}

	Function * Module::beginFunction( std::string const & name
		, spv::Id retType
		, ast::var::VariableList const & params )
	{
		spv::Id result{ getNextId() };
		m_registeredVariables.emplace( name, result );

		IdList funcTypes;
		IdList funcParams;
		funcTypes.push_back( retType );
		Function func;
		m_currentFunction = &functions.emplace_back( std::move( func ) );
		m_currentFunction->registeredVariables = m_registeredVariables; // the function has access to global scope variables.
		m_currentScopeVariables = &m_currentFunction->registeredVariables;

		for ( auto & param : params )
		{
			funcTypes.push_back( registerType( param->getType() ) );
			spv::Id paramId{ getNextId() };
			funcParams.push_back( paramId );
			debug.push_back( makeInstruction< NameInstruction >( paramId, param->getName() ) );
			auto kind = param->getType()->getKind();

			if ( isSampledImageType( kind )
				|| isImageType( kind )
				|| isSamplerType( kind )
				|| isStructType( kind )
				|| isArrayType( kind )
				|| param->isOutputParam() )
			{
				funcTypes.back() = registerPointerType( funcTypes.back(), spv::StorageClassFunction );
			}

			m_currentScopeVariables->emplace( param->getName(), funcParams.back() );
			m_registeredVariablesTypes.emplace( funcParams.back(), funcTypes.back() );
		}

		auto it = m_registeredFunctionTypes.find( funcTypes );

		if ( it == m_registeredFunctionTypes.end() )
		{
			spv::Id funcType{ getNextId() };
			globalDeclarations.push_back( makeInstruction< FunctionTypeInstruction >( funcType
				, funcTypes ) );
			it = m_registeredFunctionTypes.emplace( funcTypes, funcType ).first;
		}

		spv::Id funcType{ it->second };
		m_currentFunction->declaration.emplace_back( makeInstruction< FunctionInstruction >( retType
			, result
			, spv::Id( spv::FunctionControlMaskNone )
			, funcType ) );
		auto itType = funcTypes.begin() + 1u;
		auto itParam = funcParams.begin();

		for ( auto & param : params )
		{
			m_currentFunction->declaration.emplace_back( makeInstruction< FunctionParameterInstruction >( *itType
				, *itParam ) );
			++itType;
			++itParam;
		}

		m_registeredVariablesTypes.emplace( result, funcType );
		debug.push_back( makeInstruction< NameInstruction >( result, name ) );
		variables = &m_currentFunction->variables;

		return m_currentFunction;
	}

	Block Module::newBlock()
	{
		Block result
		{
			getNextId()
		};
		result.instructions.push_back( makeInstruction< LabelInstruction >( result.label ) );
		return result;
	}

	void Module::endFunction()
	{
		if ( m_currentFunction
			&& !m_currentFunction->cfg.blocks.empty()
			&& !m_currentFunction->variables.empty() )
		{
			auto & instructions = m_currentFunction->cfg.blocks.begin()->instructions;
			auto variables = std::move( m_currentFunction->variables );
			std::reverse( variables.begin(), variables.end() );

			for ( auto & variable : variables )
			{
				instructions.emplace( instructions.begin() + 1u
					, std::move( variable ) );
			}

			m_currentFunction->variables.clear();
		}

		variables = &globalDeclarations;
		m_currentScopeVariables = &m_registeredVariables;
		m_currentFunction = nullptr;
	}

	spv::Id Module::doRegisterNonArrayType( ast::type::TypePtr type
		, uint32_t mbrIndex
		, spv::Id parentId )
	{
		spv::Id result;

		auto unqualifiedType = getUnqualifiedType( *m_cache, type );
		auto it = m_registeredTypes.find( unqualifiedType );

		if ( it == m_registeredTypes.end() )
		{
			result = registerBaseType( unqualifiedType
				, mbrIndex
				, parentId
				, 0u );
			m_registeredTypes.emplace( unqualifiedType, result );
		}
		else
		{
			result = it->second;
		}

		return result;
	}

	spv::Id Module::registerType( ast::type::TypePtr type
		, uint32_t mbrIndex
		, spv::Id parentId
		, uint32_t arrayStride )
	{
		spv::Id result;

		if ( type->getKind() == ast::type::Kind::eArray )
		{
			auto arrayedType = std::static_pointer_cast< ast::type::Array >( type )->getType();
			auto arraySize = getArraySize( type );
			auto elementTypeId = registerType( arrayedType
				, mbrIndex
				, parentId
				, arrayStride );

			auto unqualifiedType = getUnqualifiedType( *m_cache, type );
			auto it = m_registeredTypes.find( unqualifiedType );

			if ( it == m_registeredTypes.end() )
			{
				if ( arraySize != ast::type::UnknownArraySize )
				{
					auto lengthId = registerLiteral( arraySize );
					result = getNextId();
					globalDeclarations.push_back( makeInstruction< ArrayTypeInstruction >( result
						, elementTypeId
						, lengthId ) );
				}
				else
				{
					result = getNextId();
					globalDeclarations.push_back( makeInstruction< RuntimeArrayTypeInstruction >( result
						, elementTypeId ) );
				}

				writeArrayStride( *this
					, unqualifiedType
					, result
					, arrayStride );
				m_registeredTypes.emplace( unqualifiedType, result );
			}
			else
			{
				result = it->second;
			}
		}
		else
		{
			result = doRegisterNonArrayType( type
				, mbrIndex
				, parentId );
		}

		return result;
	}

	spv::Id Module::registerBaseType( ast::type::Kind kind
		, uint32_t mbrIndex
		, spv::Id parentId
		, uint32_t arrayStride )
	{
		assert( kind != ast::type::Kind::eStruct );
		assert( kind != ast::type::Kind::eImage );
		assert( kind != ast::type::Kind::eSampledImage );

		spv::Id result{};
		auto type = m_cache->getBasicType( kind );

		if ( isVectorType( kind )
			|| isMatrixType( kind ) )
		{
			auto componentType = registerType( m_cache->getBasicType( getComponentType( kind ) ) );
			result = getNextId();

			if ( isMatrixType( kind ) )
			{
				globalDeclarations.push_back( makeInstruction< MatrixTypeInstruction >( result
					, componentType
					, getComponentCount( kind ) ) );

				if ( mbrIndex != ast::type::NotMember )
				{
					decorateMember( parentId
						, mbrIndex
						, spv::DecorationColMajor );
				}
				else
				{
					decorate( result
						, spv::DecorationColMajor );
				}
			}
			else
			{
				globalDeclarations.push_back( makeInstruction< VectorTypeInstruction >( result
					, componentType
					, getComponentCount( kind ) ) );
			}
		}
		else
		{
			result = getNextId();
			globalDeclarations.push_back( makeBaseTypeInstruction( kind, result ) );
		}

		return result;
	}

	spv::Id Module::registerBaseType( ast::type::SampledImagePtr type
		, uint32_t mbrIndex
		, spv::Id parentId )
	{
		auto imgTypeId = registerType( std::static_pointer_cast< ast::type::SampledImage >( type )->getImageType() );
		auto result = getNextId();
		globalDeclarations.push_back( makeInstruction< SampledImageTypeInstruction >( result
			, imgTypeId ) );
		return result;
	}

	spv::Id Module::registerBaseType( ast::type::ImagePtr type
		, uint32_t mbrIndex
		, spv::Id parent )
	{
		// The Sampled Type.
		auto sampledTypeId = registerType( m_cache->getBasicType( type->getConfig().sampledType ) );
		// The Image Type.
		auto result = getNextId();
		globalDeclarations.push_back( makeImageTypeInstruction( type->getConfig(), result, sampledTypeId ) );
		return result;
	}

	spv::Id Module::registerBaseType( ast::type::StructPtr type
		, uint32_t
		, spv::Id )
	{
		spv::Id result{ getNextId() };
		IdList subTypes;

		for ( auto & member : *type )
		{
			subTypes.push_back( registerType( member.type
				, member.type->getIndex()
				, result
				, member.arrayStride ) );
		}

		globalDeclarations.push_back( makeInstruction< StructTypeInstruction >( result, subTypes ) );
		debug.push_back( makeInstruction< NameInstruction >( result, type->getName() ) );

		for ( auto & member : *type )
		{
			auto index = member.type->getIndex();
			debug.push_back( makeInstruction< MemberNameInstruction >( result, index, member.name ) );
			decorateMember( result
				, index
				, makeOperands( uint32_t( spv::DecorationOffset ), member.offset ) );

			if ( isMatrixType( member.type->getKind() ) )
			{
				auto colType = getComponentType( member.type->getKind() );
				auto size = getSize( *m_cache->getBasicType( colType )
					, ast::type::MemoryLayout::eStd430 );
				decorateMember( result
					, index
					, makeOperands( uint32_t( spv::DecorationMatrixStride ), size ) );
			}
		}

		return result;
	}

	spv::Id Module::registerBaseType( ast::type::TypePtr type
		, uint32_t mbrIndex
		, spv::Id parentId
		, uint32_t arrayStride )
	{
		spv::Id result{};

		if ( type->getKind() == ast::type::Kind::eArray )
		{
			type = std::static_pointer_cast< ast::type::Array >( type )->getType();
		}

		auto kind = type->getKind();

		if ( kind == ast::type::Kind::eSampledImage )
		{
			result = registerBaseType( std::static_pointer_cast< ast::type::SampledImage >( type )
				, mbrIndex
				, parentId );
		}
		else if ( kind == ast::type::Kind::eImage )
		{
			result = registerBaseType( std::static_pointer_cast< ast::type::Image >( type )
				, mbrIndex
				, parentId );
		}
		else if ( kind == ast::type::Kind::eStruct )
		{
			result = registerBaseType( std::static_pointer_cast< ast::type::Struct >( type )
				, mbrIndex
				, parentId );
		}
		else
		{
			result = registerBaseType( kind
				, mbrIndex
				, parentId
				, arrayStride );
		}

		return result;
	}

	spv::Id Module::getNextId()
	{
		auto result = *m_currentId;
		++*m_currentId;
		return result;
	}

	//*************************************************************************
}
