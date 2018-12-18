/*
See LICENSE file in root folder
*/
#ifndef ___SDW_GlslImageAccessNames_H___
#define ___SDW_GlslImageAccessNames_H___
#pragma once

#include "CompilerSpirV/spirv/spirv.hpp"

#include <ShaderAST/Expr/EnumImageAccess.hpp>

#include <string>

namespace spirv
{
	inline spv::Op getSpirVName( ast::expr::ImageAccess value )
	{
		spv::Op result;

		switch ( value )
		{
		case ast::expr::ImageAccess::eImageSize1DF:
		case ast::expr::ImageAccess::eImageSize2DF:
		case ast::expr::ImageAccess::eImageSize3DF:
		case ast::expr::ImageAccess::eImageSizeCubeF:
		case ast::expr::ImageAccess::eImageSizeCubeArrayF:
		case ast::expr::ImageAccess::eImageSize2DRectF:
		case ast::expr::ImageAccess::eImageSize1DArrayF:
		case ast::expr::ImageAccess::eImageSize2DArrayF:
		case ast::expr::ImageAccess::eImageSizeBufferF:
		case ast::expr::ImageAccess::eImageSize2DMSF:
		case ast::expr::ImageAccess::eImageSize2DMSArrayF:
		case ast::expr::ImageAccess::eImageSize1DI:
		case ast::expr::ImageAccess::eImageSize2DI:
		case ast::expr::ImageAccess::eImageSize3DI:
		case ast::expr::ImageAccess::eImageSizeCubeI:
		case ast::expr::ImageAccess::eImageSizeCubeArrayI:
		case ast::expr::ImageAccess::eImageSize2DRectI:
		case ast::expr::ImageAccess::eImageSize1DArrayI:
		case ast::expr::ImageAccess::eImageSize2DArrayI:
		case ast::expr::ImageAccess::eImageSizeBufferI:
		case ast::expr::ImageAccess::eImageSize2DMSI:
		case ast::expr::ImageAccess::eImageSize2DMSArrayI:
		case ast::expr::ImageAccess::eImageSize1DU:
		case ast::expr::ImageAccess::eImageSize2DU:
		case ast::expr::ImageAccess::eImageSize3DU:
		case ast::expr::ImageAccess::eImageSizeCubeU:
		case ast::expr::ImageAccess::eImageSizeCubeArrayU:
		case ast::expr::ImageAccess::eImageSize2DRectU:
		case ast::expr::ImageAccess::eImageSize1DArrayU:
		case ast::expr::ImageAccess::eImageSize2DArrayU:
		case ast::expr::ImageAccess::eImageSizeBufferU:
		case ast::expr::ImageAccess::eImageSize2DMSU:
		case ast::expr::ImageAccess::eImageSize2DMSArrayU:
			result = spv::OpImageQuerySize;
			break;

		case ast::expr::ImageAccess::eImageSamples2DMSF:
		case ast::expr::ImageAccess::eImageSamples2DMSArrayF:
		case ast::expr::ImageAccess::eImageSamples2DMSI:
		case ast::expr::ImageAccess::eImageSamples2DMSArrayI:
		case ast::expr::ImageAccess::eImageSamples2DMSU:
		case ast::expr::ImageAccess::eImageSamples2DMSArrayU:
			result = spv::OpImageQuerySamples;
			break;

		case ast::expr::ImageAccess::eImageLoad1DF:
		case ast::expr::ImageAccess::eImageLoad2DF:
		case ast::expr::ImageAccess::eImageLoad3DF:
		case ast::expr::ImageAccess::eImageLoad2DRectF:
		case ast::expr::ImageAccess::eImageLoadCubeF:
		case ast::expr::ImageAccess::eImageLoadBufferF:
		case ast::expr::ImageAccess::eImageLoad1DArrayF:
		case ast::expr::ImageAccess::eImageLoad2DArrayF:
		case ast::expr::ImageAccess::eImageLoadCubeArrayF:
		case ast::expr::ImageAccess::eImageLoad2DMSF:
		case ast::expr::ImageAccess::eImageLoad2DMSArrayF:
		case ast::expr::ImageAccess::eImageLoad1DI:
		case ast::expr::ImageAccess::eImageLoad2DI:
		case ast::expr::ImageAccess::eImageLoad3DI:
		case ast::expr::ImageAccess::eImageLoad2DRectI:
		case ast::expr::ImageAccess::eImageLoadCubeI:
		case ast::expr::ImageAccess::eImageLoadBufferI:
		case ast::expr::ImageAccess::eImageLoad1DArrayI:
		case ast::expr::ImageAccess::eImageLoad2DArrayI:
		case ast::expr::ImageAccess::eImageLoadCubeArrayI:
		case ast::expr::ImageAccess::eImageLoad2DMSI:
		case ast::expr::ImageAccess::eImageLoad2DMSArrayI:
		case ast::expr::ImageAccess::eImageLoad1DU:
		case ast::expr::ImageAccess::eImageLoad2DU:
		case ast::expr::ImageAccess::eImageLoad3DU:
		case ast::expr::ImageAccess::eImageLoad2DRectU:
		case ast::expr::ImageAccess::eImageLoadCubeU:
		case ast::expr::ImageAccess::eImageLoadBufferU:
		case ast::expr::ImageAccess::eImageLoad1DArrayU:
		case ast::expr::ImageAccess::eImageLoad2DArrayU:
		case ast::expr::ImageAccess::eImageLoadCubeArrayU:
		case ast::expr::ImageAccess::eImageLoad2DMSU:
		case ast::expr::ImageAccess::eImageLoad2DMSArrayU:
			result = spv::OpImageRead;
			break;

		case ast::expr::ImageAccess::eImageAtomicAdd1DU:
		case ast::expr::ImageAccess::eImageAtomicAdd2DU:
		case ast::expr::ImageAccess::eImageAtomicAdd3DU:
		case ast::expr::ImageAccess::eImageAtomicAdd2DRectU:
		case ast::expr::ImageAccess::eImageAtomicAddCubeU:
		case ast::expr::ImageAccess::eImageAtomicAddBufferU:
		case ast::expr::ImageAccess::eImageAtomicAdd1DArrayU:
		case ast::expr::ImageAccess::eImageAtomicAdd2DArrayU:
		case ast::expr::ImageAccess::eImageAtomicAddCubeArrayU:
		case ast::expr::ImageAccess::eImageAtomicAdd2DMSU:
		case ast::expr::ImageAccess::eImageAtomicAdd2DMSArrayU:
		case ast::expr::ImageAccess::eImageAtomicAdd1DI:
		case ast::expr::ImageAccess::eImageAtomicAdd2DI:
		case ast::expr::ImageAccess::eImageAtomicAdd3DI:
		case ast::expr::ImageAccess::eImageAtomicAdd2DRectI:
		case ast::expr::ImageAccess::eImageAtomicAddCubeI:
		case ast::expr::ImageAccess::eImageAtomicAddBufferI:
		case ast::expr::ImageAccess::eImageAtomicAdd1DArrayI:
		case ast::expr::ImageAccess::eImageAtomicAdd2DArrayI:
		case ast::expr::ImageAccess::eImageAtomicAddCubeArrayI:
		case ast::expr::ImageAccess::eImageAtomicAdd2DMSI:
		case ast::expr::ImageAccess::eImageAtomicAdd2DMSArrayI:
			result = spv::OpAtomicIAdd;
			break;

		case ast::expr::ImageAccess::eImageAtomicMin1DU:
		case ast::expr::ImageAccess::eImageAtomicMin2DU:
		case ast::expr::ImageAccess::eImageAtomicMin3DU:
		case ast::expr::ImageAccess::eImageAtomicMin2DRectU:
		case ast::expr::ImageAccess::eImageAtomicMinCubeU:
		case ast::expr::ImageAccess::eImageAtomicMinBufferU:
		case ast::expr::ImageAccess::eImageAtomicMin1DArrayU:
		case ast::expr::ImageAccess::eImageAtomicMin2DArrayU:
		case ast::expr::ImageAccess::eImageAtomicMinCubeArrayU:
		case ast::expr::ImageAccess::eImageAtomicMin2DMSU:
		case ast::expr::ImageAccess::eImageAtomicMin2DMSArrayU:
			result = spv::OpAtomicUMin;
			break;

		case ast::expr::ImageAccess::eImageAtomicMin1DI:
		case ast::expr::ImageAccess::eImageAtomicMin2DI:
		case ast::expr::ImageAccess::eImageAtomicMin3DI:
		case ast::expr::ImageAccess::eImageAtomicMin2DRectI:
		case ast::expr::ImageAccess::eImageAtomicMinCubeI:
		case ast::expr::ImageAccess::eImageAtomicMinBufferI:
		case ast::expr::ImageAccess::eImageAtomicMin1DArrayI:
		case ast::expr::ImageAccess::eImageAtomicMin2DArrayI:
		case ast::expr::ImageAccess::eImageAtomicMinCubeArrayI:
		case ast::expr::ImageAccess::eImageAtomicMin2DMSI:
		case ast::expr::ImageAccess::eImageAtomicMin2DMSArrayI:
			result = spv::OpAtomicSMin;
			break;

		case ast::expr::ImageAccess::eImageAtomicMax1DU:
		case ast::expr::ImageAccess::eImageAtomicMax2DU:
		case ast::expr::ImageAccess::eImageAtomicMax3DU:
		case ast::expr::ImageAccess::eImageAtomicMax2DRectU:
		case ast::expr::ImageAccess::eImageAtomicMaxCubeU:
		case ast::expr::ImageAccess::eImageAtomicMaxBufferU:
		case ast::expr::ImageAccess::eImageAtomicMax1DArrayU:
		case ast::expr::ImageAccess::eImageAtomicMax2DArrayU:
		case ast::expr::ImageAccess::eImageAtomicMaxCubeArrayU:
		case ast::expr::ImageAccess::eImageAtomicMax2DMSU:
		case ast::expr::ImageAccess::eImageAtomicMax2DMSArrayU:
			result = spv::OpAtomicUMax;
			break;

		case ast::expr::ImageAccess::eImageAtomicMax1DI:
		case ast::expr::ImageAccess::eImageAtomicMax2DI:
		case ast::expr::ImageAccess::eImageAtomicMax3DI:
		case ast::expr::ImageAccess::eImageAtomicMax2DRectI:
		case ast::expr::ImageAccess::eImageAtomicMaxCubeI:
		case ast::expr::ImageAccess::eImageAtomicMaxBufferI:
		case ast::expr::ImageAccess::eImageAtomicMax1DArrayI:
		case ast::expr::ImageAccess::eImageAtomicMax2DArrayI:
		case ast::expr::ImageAccess::eImageAtomicMaxCubeArrayI:
		case ast::expr::ImageAccess::eImageAtomicMax2DMSI:
		case ast::expr::ImageAccess::eImageAtomicMax2DMSArrayI:
			result = spv::OpAtomicSMax;
			break;

		case ast::expr::ImageAccess::eImageAtomicAnd1DU:
		case ast::expr::ImageAccess::eImageAtomicAnd2DU:
		case ast::expr::ImageAccess::eImageAtomicAnd3DU:
		case ast::expr::ImageAccess::eImageAtomicAnd2DRectU:
		case ast::expr::ImageAccess::eImageAtomicAndCubeU:
		case ast::expr::ImageAccess::eImageAtomicAndBufferU:
		case ast::expr::ImageAccess::eImageAtomicAnd1DArrayU:
		case ast::expr::ImageAccess::eImageAtomicAnd2DArrayU:
		case ast::expr::ImageAccess::eImageAtomicAndCubeArrayU:
		case ast::expr::ImageAccess::eImageAtomicAnd2DMSU:
		case ast::expr::ImageAccess::eImageAtomicAnd2DMSArrayU:
		case ast::expr::ImageAccess::eImageAtomicAnd1DI:
		case ast::expr::ImageAccess::eImageAtomicAnd2DI:
		case ast::expr::ImageAccess::eImageAtomicAnd3DI:
		case ast::expr::ImageAccess::eImageAtomicAnd2DRectI:
		case ast::expr::ImageAccess::eImageAtomicAndCubeI:
		case ast::expr::ImageAccess::eImageAtomicAndBufferI:
		case ast::expr::ImageAccess::eImageAtomicAnd1DArrayI:
		case ast::expr::ImageAccess::eImageAtomicAnd2DArrayI:
		case ast::expr::ImageAccess::eImageAtomicAndCubeArrayI:
		case ast::expr::ImageAccess::eImageAtomicAnd2DMSI:
		case ast::expr::ImageAccess::eImageAtomicAnd2DMSArrayI:
			result = spv::OpAtomicAnd;
			break;

		case ast::expr::ImageAccess::eImageAtomicOr1DU:
		case ast::expr::ImageAccess::eImageAtomicOr2DU:
		case ast::expr::ImageAccess::eImageAtomicOr3DU:
		case ast::expr::ImageAccess::eImageAtomicOr2DRectU:
		case ast::expr::ImageAccess::eImageAtomicOrCubeU:
		case ast::expr::ImageAccess::eImageAtomicOrBufferU:
		case ast::expr::ImageAccess::eImageAtomicOr1DArrayU:
		case ast::expr::ImageAccess::eImageAtomicOr2DArrayU:
		case ast::expr::ImageAccess::eImageAtomicOrCubeArrayU:
		case ast::expr::ImageAccess::eImageAtomicOr2DMSU:
		case ast::expr::ImageAccess::eImageAtomicOr2DMSArrayU:
		case ast::expr::ImageAccess::eImageAtomicOr1DI:
		case ast::expr::ImageAccess::eImageAtomicOr2DI:
		case ast::expr::ImageAccess::eImageAtomicOr3DI:
		case ast::expr::ImageAccess::eImageAtomicOr2DRectI:
		case ast::expr::ImageAccess::eImageAtomicOrCubeI:
		case ast::expr::ImageAccess::eImageAtomicOrBufferI:
		case ast::expr::ImageAccess::eImageAtomicOr1DArrayI:
		case ast::expr::ImageAccess::eImageAtomicOr2DArrayI:
		case ast::expr::ImageAccess::eImageAtomicOrCubeArrayI:
		case ast::expr::ImageAccess::eImageAtomicOr2DMSI:
		case ast::expr::ImageAccess::eImageAtomicOr2DMSArrayI:
			result = spv::OpAtomicOr;
			break;

		case ast::expr::ImageAccess::eImageAtomicXor1DU:
		case ast::expr::ImageAccess::eImageAtomicXor2DU:
		case ast::expr::ImageAccess::eImageAtomicXor3DU:
		case ast::expr::ImageAccess::eImageAtomicXor2DRectU:
		case ast::expr::ImageAccess::eImageAtomicXorCubeU:
		case ast::expr::ImageAccess::eImageAtomicXorBufferU:
		case ast::expr::ImageAccess::eImageAtomicXor1DArrayU:
		case ast::expr::ImageAccess::eImageAtomicXor2DArrayU:
		case ast::expr::ImageAccess::eImageAtomicXorCubeArrayU:
		case ast::expr::ImageAccess::eImageAtomicXor2DMSU:
		case ast::expr::ImageAccess::eImageAtomicXor2DMSArrayU:
		case ast::expr::ImageAccess::eImageAtomicXor1DI:
		case ast::expr::ImageAccess::eImageAtomicXor2DI:
		case ast::expr::ImageAccess::eImageAtomicXor3DI:
		case ast::expr::ImageAccess::eImageAtomicXor2DRectI:
		case ast::expr::ImageAccess::eImageAtomicXorCubeI:
		case ast::expr::ImageAccess::eImageAtomicXorBufferI:
		case ast::expr::ImageAccess::eImageAtomicXor1DArrayI:
		case ast::expr::ImageAccess::eImageAtomicXor2DArrayI:
		case ast::expr::ImageAccess::eImageAtomicXorCubeArrayI:
		case ast::expr::ImageAccess::eImageAtomicXor2DMSI:
		case ast::expr::ImageAccess::eImageAtomicXor2DMSArrayI:
			result = spv::OpAtomicXor;
			break;

		case ast::expr::ImageAccess::eImageAtomicExchange1DU:
		case ast::expr::ImageAccess::eImageAtomicExchange2DU:
		case ast::expr::ImageAccess::eImageAtomicExchange3DU:
		case ast::expr::ImageAccess::eImageAtomicExchange2DRectU:
		case ast::expr::ImageAccess::eImageAtomicExchangeCubeU:
		case ast::expr::ImageAccess::eImageAtomicExchangeBufferU:
		case ast::expr::ImageAccess::eImageAtomicExchange1DArrayU:
		case ast::expr::ImageAccess::eImageAtomicExchange2DArrayU:
		case ast::expr::ImageAccess::eImageAtomicExchangeCubeArrayU:
		case ast::expr::ImageAccess::eImageAtomicExchange2DMSU:
		case ast::expr::ImageAccess::eImageAtomicExchange2DMSArrayU:
		case ast::expr::ImageAccess::eImageAtomicExchange1DI:
		case ast::expr::ImageAccess::eImageAtomicExchange2DI:
		case ast::expr::ImageAccess::eImageAtomicExchange3DI:
		case ast::expr::ImageAccess::eImageAtomicExchange2DRectI:
		case ast::expr::ImageAccess::eImageAtomicExchangeCubeI:
		case ast::expr::ImageAccess::eImageAtomicExchangeBufferI:
		case ast::expr::ImageAccess::eImageAtomicExchange1DArrayI:
		case ast::expr::ImageAccess::eImageAtomicExchange2DArrayI:
		case ast::expr::ImageAccess::eImageAtomicExchangeCubeArrayI:
		case ast::expr::ImageAccess::eImageAtomicExchange2DMSI:
		case ast::expr::ImageAccess::eImageAtomicExchange2DMSArrayI:
			result = spv::OpAtomicExchange;
			break;

		case ast::expr::ImageAccess::eImageAtomicCompSwap1DU:
		case ast::expr::ImageAccess::eImageAtomicCompSwap2DU:
		case ast::expr::ImageAccess::eImageAtomicCompSwap3DU:
		case ast::expr::ImageAccess::eImageAtomicCompSwap2DRectU:
		case ast::expr::ImageAccess::eImageAtomicCompSwapCubeU:
		case ast::expr::ImageAccess::eImageAtomicCompSwapBufferU:
		case ast::expr::ImageAccess::eImageAtomicCompSwap1DArrayU:
		case ast::expr::ImageAccess::eImageAtomicCompSwap2DArrayU:
		case ast::expr::ImageAccess::eImageAtomicCompSwapCubeArrayU:
		case ast::expr::ImageAccess::eImageAtomicCompSwap2DMSU:
		case ast::expr::ImageAccess::eImageAtomicCompSwap2DMSArrayU:
		case ast::expr::ImageAccess::eImageAtomicCompSwap1DI:
		case ast::expr::ImageAccess::eImageAtomicCompSwap2DI:
		case ast::expr::ImageAccess::eImageAtomicCompSwap3DI:
		case ast::expr::ImageAccess::eImageAtomicCompSwap2DRectI:
		case ast::expr::ImageAccess::eImageAtomicCompSwapCubeI:
		case ast::expr::ImageAccess::eImageAtomicCompSwapBufferI:
		case ast::expr::ImageAccess::eImageAtomicCompSwap1DArrayI:
		case ast::expr::ImageAccess::eImageAtomicCompSwap2DArrayI:
		case ast::expr::ImageAccess::eImageAtomicCompSwapCubeArrayI:
		case ast::expr::ImageAccess::eImageAtomicCompSwap2DMSI:
		case ast::expr::ImageAccess::eImageAtomicCompSwap2DMSArrayI:
			result = spv::OpAtomicCompareExchange;
			break;

		default:
			throw std::runtime_error{ "Unsupported ImageAccess type." };
		}

		return result;
	}
}

#endif
