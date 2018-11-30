/*
See LICENSE file in root folder
*/
#ifndef ___SDW_SpirvOpHelpers_H___
#define ___SDW_SpirvOpHelpers_H___
#pragma once

#include "SpirvModule.hpp"

#include <string>
#include <vector>

namespace spirv
{
	uint16_t getOpBaseSize( spv::Op op );
	std::string getOpName( spv::Op op );
	spv::Op getOpCode( ast::type::Kind kind );
	spv::Op getOpCode( ast::expr::Kind exprKind
		, ast::type::Kind typeKind );
	spv::Op getBinOpCode( ast::expr::Kind exprKind
		, ast::type::Kind lhsTypeKind
		, ast::type::Kind rhsTypeKind
		, bool & switchParams );
	spv::Op getOpCode( ast::type::TypePtr type );
}

#endif
