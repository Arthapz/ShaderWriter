/*
See LICENSE file in root folder
*/
#ifndef ___SDW_MaybeOptionalMat3x4_H___
#define ___SDW_MaybeOptionalMat3x4_H___
#pragma once

#include "ShaderWriter/Optional/OptionalMat3x4.hpp"

namespace sdw
{
	template< typename ValueT >
	struct MaybeOptional< Mat3x4T< ValueT > >
		: public Mat3x4T< ValueT >
	{
		using MyValue = Mat3x4T< ValueT >;

		using ValueType = ValueT;
		using my_vec = MaybeOptional< Vec4T< ValueT > >;
		using my_mat = MaybeOptional< Mat3x4T< ValueT > >;

		inline MaybeOptional( Shader * shader
			, expr::ExprPtr expr );
		inline MaybeOptional( Shader * shader
			, expr::ExprPtr expr
			, bool enabled );
		inline MaybeOptional( MyValue const & rhs );
		inline MaybeOptional( Optional< MyValue > const & rhs );
		inline MaybeOptional( MaybeOptional< MyValue > const & rhs );

		inline MaybeOptional< Mat3x4T< ValueT > > & operator=( MaybeOptional< Mat3x4T< ValueT > > const & rhs );
		template< typename IndexT >
		inline MaybeOptional< Vec4T< ValueT > > operator[]( IndexT const & rhs )const;

		inline operator MyValue()const;
		inline operator Optional< MyValue >()const;

		inline bool isOptional()const;
		inline bool isEnabled()const;

	private:
		bool m_optional;
		bool m_enabled;
	};
}

#include "MaybeOptionalMat3x4.inl"

#endif
