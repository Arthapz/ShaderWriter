/*
See LICENSE file in root folder
*/
#ifndef ___AST_StmtElse_H___
#define ___AST_StmtElse_H___
#pragma once

#include "StmtCompound.hpp"

namespace ast::stmt
{
	class If;
	class Else
		: public Compound
	{
		friend class If;

	private:
		Else();

	public:
		void accept( VisitorPtr vis )override;
	};
	using ElsePtr = std::unique_ptr< Else >;
}

#endif
