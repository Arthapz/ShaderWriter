/*
See LICENSE file in root folder
*/
#include "ShaderWriter/SPIRV/SpirvExprVisitor.hpp"

#include "ShaderWriter/SPIRV/SpirvHelpers.hpp"
#include "ShaderWriter/SPIRV/SpirvImageAccessNames.hpp"
#include "ShaderWriter/SPIRV/SpirvIntrinsicNames.hpp"
#include "ShaderWriter/SPIRV/SpirvTextureAccessNames.hpp"

#include <sstream>

namespace sdw::spirv
{
	namespace
	{
		bool isCtor( std::string const & name )
		{
			return name == "bvec2"
				|| name == "bvec3"
				|| name == "bvec4"
				|| name == "ivec2"
				|| name == "ivec3"
				|| name == "ivec4"
				|| name == "uvec2"
				|| name == "uvec3"
				|| name == "uvec4"
				|| name == "dvec2"
				|| name == "dvec3"
				|| name == "dvec4"
				|| name == "vec2"
				|| name == "vec3"
				|| name == "vec4"
				|| name == "dmat2"
				|| name == "dmat2x3"
				|| name == "dmat2x4"
				|| name == "dmat3x2"
				|| name == "dmat3"
				|| name == "dmat3x4"
				|| name == "dmat4x2"
				|| name == "dmat4x3"
				|| name == "dmat4"
				|| name == "mat2"
				|| name == "mat2x3"
				|| name == "mat2x4"
				|| name == "mat3x2"
				|| name == "mat3"
				|| name == "mat3x4"
				|| name == "mat4x2"
				|| name == "mat4x3"
				|| name == "mat4";
		}

		std::vector< uint32_t > getSwizzleComponents( expr::SwizzleKind kind )
		{
			switch ( kind )
			{
			case ast::expr::SwizzleKind::e0:
				return { 0 };
			case ast::expr::SwizzleKind::e1:
				return { 1 };
			case ast::expr::SwizzleKind::e2:
				return { 2 };
			case ast::expr::SwizzleKind::e3:
				return { 3 };
			case ast::expr::SwizzleKind::e00:
				return { 0, 0 };
			case ast::expr::SwizzleKind::e01:
				return { 0, 1 };
			case ast::expr::SwizzleKind::e02:
				return { 0, 2 };
			case ast::expr::SwizzleKind::e03:
				return { 0, 3 };
			case ast::expr::SwizzleKind::e10:
				return { 1, 0 };
			case ast::expr::SwizzleKind::e11:
				return { 1, 1 };
			case ast::expr::SwizzleKind::e12:
				return { 1, 2 };
			case ast::expr::SwizzleKind::e13:
				return { 1, 3 };
			case ast::expr::SwizzleKind::e20:
				return { 2, 0 };
			case ast::expr::SwizzleKind::e21:
				return { 2, 1 };
			case ast::expr::SwizzleKind::e22:
				return { 2, 2 };
			case ast::expr::SwizzleKind::e23:
				return { 2, 3 };
			case ast::expr::SwizzleKind::e30:
				return { 3, 0 };
			case ast::expr::SwizzleKind::e31:
				return { 3, 1 };
			case ast::expr::SwizzleKind::e32:
				return { 3, 2 };
			case ast::expr::SwizzleKind::e33:
				return { 3, 3 };
			case ast::expr::SwizzleKind::e000:
				return { 0, 0, 0 };
			case ast::expr::SwizzleKind::e001:
				return { 0, 0, 1 };
			case ast::expr::SwizzleKind::e002:
				return { 0, 0, 2 };
			case ast::expr::SwizzleKind::e003:
				return { 0, 0, 3 };
			case ast::expr::SwizzleKind::e010:
				return { 0, 1, 0 };
			case ast::expr::SwizzleKind::e011:
				return { 0, 1, 1 };
			case ast::expr::SwizzleKind::e012:
				return { 0, 1, 2 };
			case ast::expr::SwizzleKind::e013:
				return { 0, 1, 3 };
			case ast::expr::SwizzleKind::e020:
				return { 0, 2, 0 };
			case ast::expr::SwizzleKind::e021:
				return { 0, 2, 1 };
			case ast::expr::SwizzleKind::e022:
				return { 0, 2, 2 };
			case ast::expr::SwizzleKind::e023:
				return { 0, 2, 3 };
			case ast::expr::SwizzleKind::e030:
				return { 0, 3, 0 };
			case ast::expr::SwizzleKind::e031:
				return { 0, 3, 1 };
			case ast::expr::SwizzleKind::e032:
				return { 0, 3, 2 };
			case ast::expr::SwizzleKind::e033:
				return { 0, 3, 3 };
			case ast::expr::SwizzleKind::e100:
				return { 1, 0, 0 };
			case ast::expr::SwizzleKind::e101:
				return { 1, 0, 1 };
			case ast::expr::SwizzleKind::e102:
				return { 1, 0, 2 };
			case ast::expr::SwizzleKind::e103:
				return { 1, 0, 3 };
			case ast::expr::SwizzleKind::e110:
				return { 1, 1, 0 };
			case ast::expr::SwizzleKind::e111:
				return { 1, 1, 1 };
			case ast::expr::SwizzleKind::e112:
				return { 1, 1, 2 };
			case ast::expr::SwizzleKind::e113:
				return { 1, 1, 3 };
			case ast::expr::SwizzleKind::e120:
				return { 1, 2, 0 };
			case ast::expr::SwizzleKind::e121:
				return { 1, 2, 1 };
			case ast::expr::SwizzleKind::e122:
				return { 1, 2, 2 };
			case ast::expr::SwizzleKind::e123:
				return { 1, 2, 3 };
			case ast::expr::SwizzleKind::e130:
				return { 1, 3, 0 };
			case ast::expr::SwizzleKind::e131:
				return { 1, 3, 1 };
			case ast::expr::SwizzleKind::e132:
				return { 1, 3, 2 };
			case ast::expr::SwizzleKind::e133:
				return { 1, 3, 3 };
			case ast::expr::SwizzleKind::e200:
				return { 2, 0, 0 };
			case ast::expr::SwizzleKind::e201:
				return { 2, 0, 1 };
			case ast::expr::SwizzleKind::e202:
				return { 2, 0, 2 };
			case ast::expr::SwizzleKind::e203:
				return { 2, 0, 3 };
			case ast::expr::SwizzleKind::e210:
				return { 2, 1, 0 };
			case ast::expr::SwizzleKind::e211:
				return { 2, 1, 1 };
			case ast::expr::SwizzleKind::e212:
				return { 2, 1, 2 };
			case ast::expr::SwizzleKind::e213:
				return { 2, 1, 3 };
			case ast::expr::SwizzleKind::e220:
				return { 2, 2, 0 };
			case ast::expr::SwizzleKind::e221:
				return { 2, 2, 1 };
			case ast::expr::SwizzleKind::e222:
				return { 2, 2, 2 };
			case ast::expr::SwizzleKind::e223:
				return { 2, 2, 3 };
			case ast::expr::SwizzleKind::e230:
				return { 2, 3, 0 };
			case ast::expr::SwizzleKind::e231:
				return { 2, 3, 1 };
			case ast::expr::SwizzleKind::e232:
				return { 2, 3, 2 };
			case ast::expr::SwizzleKind::e233:
				return { 2, 3, 3 };
			case ast::expr::SwizzleKind::e300:
				return { 3, 0, 0 };
			case ast::expr::SwizzleKind::e301:
				return { 3, 0, 1 };
			case ast::expr::SwizzleKind::e302:
				return { 3, 0, 2 };
			case ast::expr::SwizzleKind::e303:
				return { 3, 0, 3 };
			case ast::expr::SwizzleKind::e310:
				return { 3, 1, 0 };
			case ast::expr::SwizzleKind::e311:
				return { 3, 1, 1 };
			case ast::expr::SwizzleKind::e312:
				return { 3, 1, 2 };
			case ast::expr::SwizzleKind::e313:
				return { 3, 1, 3 };
			case ast::expr::SwizzleKind::e320:
				return { 3, 2, 0 };
			case ast::expr::SwizzleKind::e321:
				return { 3, 2, 1 };
			case ast::expr::SwizzleKind::e322:
				return { 3, 2, 2 };
			case ast::expr::SwizzleKind::e323:
				return { 3, 2, 3 };
			case ast::expr::SwizzleKind::e330:
				return { 3, 3, 0 };
			case ast::expr::SwizzleKind::e331:
				return { 3, 3, 1 };
			case ast::expr::SwizzleKind::e332:
				return { 3, 3, 2 };
			case ast::expr::SwizzleKind::e333:
				return { 3, 3, 3 };
			case ast::expr::SwizzleKind::e0000:
				return { 0, 0, 0, 0 };
			case ast::expr::SwizzleKind::e0001:
				return { 0, 0, 0, 1 };
			case ast::expr::SwizzleKind::e0002:
				return { 0, 0, 0, 2 };
			case ast::expr::SwizzleKind::e0003:
				return { 0, 0, 0, 3 };
			case ast::expr::SwizzleKind::e0010:
				return { 0, 0, 1, 0 };
			case ast::expr::SwizzleKind::e0011:
				return { 0, 0, 1, 1 };
			case ast::expr::SwizzleKind::e0012:
				return { 0, 0, 1, 2 };
			case ast::expr::SwizzleKind::e0013:
				return { 0, 0, 1, 3 };
			case ast::expr::SwizzleKind::e0020:
				return { 0, 0, 2, 0 };
			case ast::expr::SwizzleKind::e0021:
				return { 0, 0, 2, 1 };
			case ast::expr::SwizzleKind::e0022:
				return { 0, 0, 2, 2 };
			case ast::expr::SwizzleKind::e0023:
				return { 0, 0, 2, 3 };
			case ast::expr::SwizzleKind::e0030:
				return { 0, 0, 3, 0 };
			case ast::expr::SwizzleKind::e0031:
				return { 0, 0, 3, 1 };
			case ast::expr::SwizzleKind::e0032:
				return { 0, 0, 3, 2 };
			case ast::expr::SwizzleKind::e0033:
				return { 0, 0, 3, 3 };
			case ast::expr::SwizzleKind::e0100:
				return { 0, 1, 0, 0 };
			case ast::expr::SwizzleKind::e0101:
				return { 0, 1, 0, 1 };
			case ast::expr::SwizzleKind::e0102:
				return { 0, 1, 0, 2 };
			case ast::expr::SwizzleKind::e0103:
				return { 0, 1, 0, 3 };
			case ast::expr::SwizzleKind::e0110:
				return { 0, 1, 1, 0 };
			case ast::expr::SwizzleKind::e0111:
				return { 0, 1, 1, 1 };
			case ast::expr::SwizzleKind::e0112:
				return { 0, 1, 1, 2 };
			case ast::expr::SwizzleKind::e0113:
				return { 0, 1, 1, 3 };
			case ast::expr::SwizzleKind::e0120:
				return { 0, 1, 2, 0 };
			case ast::expr::SwizzleKind::e0121:
				return { 0, 1, 2, 1 };
			case ast::expr::SwizzleKind::e0122:
				return { 0, 1, 2, 2 };
			case ast::expr::SwizzleKind::e0123:
				return { 0, 1, 2, 3 };
			case ast::expr::SwizzleKind::e0130:
				return { 0, 1, 3, 0 };
			case ast::expr::SwizzleKind::e0131:
				return { 0, 1, 3, 1 };
			case ast::expr::SwizzleKind::e0132:
				return { 0, 1, 3, 2 };
			case ast::expr::SwizzleKind::e0133:
				return { 0, 1, 3, 3 };
			case ast::expr::SwizzleKind::e0200:
				return { 0, 2, 0, 0 };
			case ast::expr::SwizzleKind::e0201:
				return { 0, 2, 0, 1 };
			case ast::expr::SwizzleKind::e0202:
				return { 0, 2, 0, 2 };
			case ast::expr::SwizzleKind::e0203:
				return { 0, 2, 0, 3 };
			case ast::expr::SwizzleKind::e0210:
				return { 0, 2, 1, 0 };
			case ast::expr::SwizzleKind::e0211:
				return { 0, 2, 1, 1 };
			case ast::expr::SwizzleKind::e0212:
				return { 0, 2, 1, 2 };
			case ast::expr::SwizzleKind::e0213:
				return { 0, 2, 1, 3 };
			case ast::expr::SwizzleKind::e0220:
				return { 0, 2, 2, 0 };
			case ast::expr::SwizzleKind::e0221:
				return { 0, 2, 2, 1 };
			case ast::expr::SwizzleKind::e0222:
				return { 0, 2, 2, 2 };
			case ast::expr::SwizzleKind::e0223:
				return { 0, 2, 2, 3 };
			case ast::expr::SwizzleKind::e0230:
				return { 0, 2, 3, 0 };
			case ast::expr::SwizzleKind::e0231:
				return { 0, 2, 3, 1 };
			case ast::expr::SwizzleKind::e0232:
				return { 0, 2, 3, 2 };
			case ast::expr::SwizzleKind::e0233:
				return { 0, 2, 3, 3 };
			case ast::expr::SwizzleKind::e0300:
				return { 0, 3, 0, 0 };
			case ast::expr::SwizzleKind::e0301:
				return { 0, 3, 0, 1 };
			case ast::expr::SwizzleKind::e0302:
				return { 0, 3, 0, 2 };
			case ast::expr::SwizzleKind::e0303:
				return { 0, 3, 0, 3 };
			case ast::expr::SwizzleKind::e0310:
				return { 0, 3, 1, 0 };
			case ast::expr::SwizzleKind::e0311:
				return { 0, 3, 1, 1 };
			case ast::expr::SwizzleKind::e0312:
				return { 0, 3, 1, 2 };
			case ast::expr::SwizzleKind::e0313:
				return { 0, 3, 1, 3 };
			case ast::expr::SwizzleKind::e0320:
				return { 0, 3, 2, 0 };
			case ast::expr::SwizzleKind::e0321:
				return { 0, 3, 2, 1 };
			case ast::expr::SwizzleKind::e0322:
				return { 0, 3, 2, 2 };
			case ast::expr::SwizzleKind::e0323:
				return { 0, 3, 2, 3 };
			case ast::expr::SwizzleKind::e0330:
				return { 0, 3, 3, 0 };
			case ast::expr::SwizzleKind::e0331:
				return { 0, 3, 3, 1 };
			case ast::expr::SwizzleKind::e0332:
				return { 0, 3, 3, 2 };
			case ast::expr::SwizzleKind::e0333:
				return { 0, 3, 3, 3 };
			case ast::expr::SwizzleKind::e1000:
				return { 1, 0, 0, 0 };
			case ast::expr::SwizzleKind::e1001:
				return { 1, 0, 0, 1 };
			case ast::expr::SwizzleKind::e1002:
				return { 1, 0, 0, 2 };
			case ast::expr::SwizzleKind::e1003:
				return { 1, 0, 0, 3 };
			case ast::expr::SwizzleKind::e1010:
				return { 1, 0, 1, 0 };
			case ast::expr::SwizzleKind::e1011:
				return { 1, 0, 1, 1 };
			case ast::expr::SwizzleKind::e1012:
				return { 1, 0, 1, 2 };
			case ast::expr::SwizzleKind::e1013:
				return { 1, 0, 1, 3 };
			case ast::expr::SwizzleKind::e1020:
				return { 1, 0, 2, 0 };
			case ast::expr::SwizzleKind::e1021:
				return { 1, 0, 2, 1 };
			case ast::expr::SwizzleKind::e1022:
				return { 1, 0, 2, 2 };
			case ast::expr::SwizzleKind::e1023:
				return { 1, 0, 2, 3 };
			case ast::expr::SwizzleKind::e1030:
				return { 1, 0, 3, 0 };
			case ast::expr::SwizzleKind::e1031:
				return { 1, 0, 3, 1 };
			case ast::expr::SwizzleKind::e1032:
				return { 1, 0, 3, 2 };
			case ast::expr::SwizzleKind::e1033:
				return { 1, 0, 3, 3 };
			case ast::expr::SwizzleKind::e1100:
				return { 1, 1, 0, 0 };
			case ast::expr::SwizzleKind::e1101:
				return { 1, 1, 0, 1 };
			case ast::expr::SwizzleKind::e1102:
				return { 1, 1, 0, 2 };
			case ast::expr::SwizzleKind::e1103:
				return { 1, 1, 0, 3 };
			case ast::expr::SwizzleKind::e1110:
				return { 1, 1, 1, 0 };
			case ast::expr::SwizzleKind::e1111:
				return { 1, 1, 1, 1 };
			case ast::expr::SwizzleKind::e1112:
				return { 1, 1, 1, 2 };
			case ast::expr::SwizzleKind::e1113:
				return { 1, 1, 1, 3 };
			case ast::expr::SwizzleKind::e1120:
				return { 1, 1, 2, 0 };
			case ast::expr::SwizzleKind::e1121:
				return { 1, 1, 2, 1 };
			case ast::expr::SwizzleKind::e1122:
				return { 1, 1, 2, 2 };
			case ast::expr::SwizzleKind::e1123:
				return { 1, 1, 2, 3 };
			case ast::expr::SwizzleKind::e1130:
				return { 1, 1, 3, 0 };
			case ast::expr::SwizzleKind::e1131:
				return { 1, 1, 3, 1 };
			case ast::expr::SwizzleKind::e1132:
				return { 1, 1, 3, 2 };
			case ast::expr::SwizzleKind::e1133:
				return { 1, 1, 3, 3 };
			case ast::expr::SwizzleKind::e1200:
				return { 1, 2, 0, 0 };
			case ast::expr::SwizzleKind::e1201:
				return { 1, 2, 0, 1 };
			case ast::expr::SwizzleKind::e1202:
				return { 1, 2, 0, 2 };
			case ast::expr::SwizzleKind::e1203:
				return { 1, 2, 0, 3 };
			case ast::expr::SwizzleKind::e1210:
				return { 1, 2, 1, 0 };
			case ast::expr::SwizzleKind::e1211:
				return { 1, 2, 1, 1 };
			case ast::expr::SwizzleKind::e1212:
				return { 1, 2, 1, 2 };
			case ast::expr::SwizzleKind::e1213:
				return { 1, 2, 1, 3 };
			case ast::expr::SwizzleKind::e1220:
				return { 1, 2, 2, 0 };
			case ast::expr::SwizzleKind::e1221:
				return { 1, 2, 2, 1 };
			case ast::expr::SwizzleKind::e1222:
				return { 1, 2, 2, 2 };
			case ast::expr::SwizzleKind::e1223:
				return { 1, 2, 2, 3 };
			case ast::expr::SwizzleKind::e1230:
				return { 1, 2, 3, 0 };
			case ast::expr::SwizzleKind::e1231:
				return { 1, 2, 3, 1 };
			case ast::expr::SwizzleKind::e1232:
				return { 1, 2, 3, 2 };
			case ast::expr::SwizzleKind::e1233:
				return { 1, 2, 3, 3 };
			case ast::expr::SwizzleKind::e1300:
				return { 1, 3, 0, 0 };
			case ast::expr::SwizzleKind::e1301:
				return { 1, 3, 0, 1 };
			case ast::expr::SwizzleKind::e1302:
				return { 1, 3, 0, 2 };
			case ast::expr::SwizzleKind::e1303:
				return { 1, 3, 0, 3 };
			case ast::expr::SwizzleKind::e1310:
				return { 1, 3, 1, 0 };
			case ast::expr::SwizzleKind::e1311:
				return { 1, 3, 1, 1 };
			case ast::expr::SwizzleKind::e1312:
				return { 1, 3, 1, 2 };
			case ast::expr::SwizzleKind::e1313:
				return { 1, 3, 1, 3 };
			case ast::expr::SwizzleKind::e1320:
				return { 1, 3, 2, 0 };
			case ast::expr::SwizzleKind::e1321:
				return { 1, 3, 2, 1 };
			case ast::expr::SwizzleKind::e1322:
				return { 1, 3, 2, 2 };
			case ast::expr::SwizzleKind::e1323:
				return { 1, 3, 2, 3 };
			case ast::expr::SwizzleKind::e1330:
				return { 1, 3, 3, 0 };
			case ast::expr::SwizzleKind::e1331:
				return { 1, 3, 3, 1 };
			case ast::expr::SwizzleKind::e1332:
				return { 1, 3, 3, 2 };
			case ast::expr::SwizzleKind::e1333:
				return { 1, 3, 3, 3 };
			case ast::expr::SwizzleKind::e2000:
				return { 2, 0, 0, 0 };
			case ast::expr::SwizzleKind::e2001:
				return { 2, 0, 0, 1 };
			case ast::expr::SwizzleKind::e2002:
				return { 2, 0, 0, 2 };
			case ast::expr::SwizzleKind::e2003:
				return { 2, 0, 0, 3 };
			case ast::expr::SwizzleKind::e2010:
				return { 2, 0, 1, 0 };
			case ast::expr::SwizzleKind::e2011:
				return { 2, 0, 1, 1 };
			case ast::expr::SwizzleKind::e2012:
				return { 2, 0, 1, 2 };
			case ast::expr::SwizzleKind::e2013:
				return { 2, 0, 1, 3 };
			case ast::expr::SwizzleKind::e2020:
				return { 2, 0, 2, 0 };
			case ast::expr::SwizzleKind::e2021:
				return { 2, 0, 2, 1 };
			case ast::expr::SwizzleKind::e2022:
				return { 2, 0, 2, 2 };
			case ast::expr::SwizzleKind::e2023:
				return { 2, 0, 2, 3 };
			case ast::expr::SwizzleKind::e2030:
				return { 2, 0, 3, 0 };
			case ast::expr::SwizzleKind::e2031:
				return { 2, 0, 3, 1 };
			case ast::expr::SwizzleKind::e2032:
				return { 2, 0, 3, 2 };
			case ast::expr::SwizzleKind::e2033:
				return { 2, 0, 3, 3 };
			case ast::expr::SwizzleKind::e2100:
				return { 2, 1, 0, 0 };
			case ast::expr::SwizzleKind::e2101:
				return { 2, 1, 0, 1 };
			case ast::expr::SwizzleKind::e2102:
				return { 2, 1, 0, 2 };
			case ast::expr::SwizzleKind::e2103:
				return { 2, 1, 0, 3 };
			case ast::expr::SwizzleKind::e2110:
				return { 2, 1, 1, 0 };
			case ast::expr::SwizzleKind::e2111:
				return { 2, 1, 1, 1 };
			case ast::expr::SwizzleKind::e2112:
				return { 2, 1, 1, 2 };
			case ast::expr::SwizzleKind::e2113:
				return { 2, 1, 1, 3 };
			case ast::expr::SwizzleKind::e2120:
				return { 2, 1, 2, 0 };
			case ast::expr::SwizzleKind::e2121:
				return { 2, 1, 2, 1 };
			case ast::expr::SwizzleKind::e2122:
				return { 2, 1, 2, 2 };
			case ast::expr::SwizzleKind::e2123:
				return { 2, 1, 2, 3 };
			case ast::expr::SwizzleKind::e2130:
				return { 2, 1, 3, 0 };
			case ast::expr::SwizzleKind::e2131:
				return { 2, 1, 3, 1 };
			case ast::expr::SwizzleKind::e2132:
				return { 2, 1, 3, 2 };
			case ast::expr::SwizzleKind::e2133:
				return { 2, 1, 3, 3 };
			case ast::expr::SwizzleKind::e2200:
				return { 2, 2, 0, 0 };
			case ast::expr::SwizzleKind::e2201:
				return { 2, 2, 0, 1 };
			case ast::expr::SwizzleKind::e2202:
				return { 2, 2, 0, 2 };
			case ast::expr::SwizzleKind::e2203:
				return { 2, 2, 0, 3 };
			case ast::expr::SwizzleKind::e2210:
				return { 2, 2, 1, 0 };
			case ast::expr::SwizzleKind::e2211:
				return { 2, 2, 1, 1 };
			case ast::expr::SwizzleKind::e2212:
				return { 2, 2, 1, 2 };
			case ast::expr::SwizzleKind::e2213:
				return { 2, 2, 1, 3 };
			case ast::expr::SwizzleKind::e2220:
				return { 2, 2, 2, 0 };
			case ast::expr::SwizzleKind::e2221:
				return { 2, 2, 2, 1 };
			case ast::expr::SwizzleKind::e2222:
				return { 2, 2, 2, 2 };
			case ast::expr::SwizzleKind::e2223:
				return { 2, 2, 2, 3 };
			case ast::expr::SwizzleKind::e2230:
				return { 2, 2, 3, 0 };
			case ast::expr::SwizzleKind::e2231:
				return { 2, 2, 3, 1 };
			case ast::expr::SwizzleKind::e2232:
				return { 2, 2, 3, 2 };
			case ast::expr::SwizzleKind::e2233:
				return { 2, 2, 3, 3 };
			case ast::expr::SwizzleKind::e2300:
				return { 2, 3, 0, 0 };
			case ast::expr::SwizzleKind::e2301:
				return { 2, 3, 0, 1 };
			case ast::expr::SwizzleKind::e2302:
				return { 2, 3, 0, 2 };
			case ast::expr::SwizzleKind::e2303:
				return { 2, 3, 0, 3 };
			case ast::expr::SwizzleKind::e2310:
				return { 2, 3, 1, 0 };
			case ast::expr::SwizzleKind::e2311:
				return { 2, 3, 1, 1 };
			case ast::expr::SwizzleKind::e2312:
				return { 2, 3, 1, 2 };
			case ast::expr::SwizzleKind::e2313:
				return { 2, 3, 1, 3 };
			case ast::expr::SwizzleKind::e2320:
				return { 2, 3, 2, 0 };
			case ast::expr::SwizzleKind::e2321:
				return { 2, 3, 2, 1 };
			case ast::expr::SwizzleKind::e2322:
				return { 2, 3, 2, 2 };
			case ast::expr::SwizzleKind::e2323:
				return { 2, 3, 2, 3 };
			case ast::expr::SwizzleKind::e2330:
				return { 2, 3, 3, 0 };
			case ast::expr::SwizzleKind::e2331:
				return { 2, 3, 3, 1 };
			case ast::expr::SwizzleKind::e2332:
				return { 2, 3, 3, 2 };
			case ast::expr::SwizzleKind::e2333:
				return { 2, 3, 3, 3 };
			case ast::expr::SwizzleKind::e3000:
				return { 3, 0, 0, 0 };
			case ast::expr::SwizzleKind::e3001:
				return { 3, 0, 0, 1 };
			case ast::expr::SwizzleKind::e3002:
				return { 3, 0, 0, 2 };
			case ast::expr::SwizzleKind::e3003:
				return { 3, 0, 0, 3 };
			case ast::expr::SwizzleKind::e3010:
				return { 3, 0, 1, 0 };
			case ast::expr::SwizzleKind::e3011:
				return { 3, 0, 1, 1 };
			case ast::expr::SwizzleKind::e3012:
				return { 3, 0, 1, 2 };
			case ast::expr::SwizzleKind::e3013:
				return { 3, 0, 1, 3 };
			case ast::expr::SwizzleKind::e3020:
				return { 3, 0, 2, 0 };
			case ast::expr::SwizzleKind::e3021:
				return { 3, 0, 2, 1 };
			case ast::expr::SwizzleKind::e3022:
				return { 3, 0, 2, 2 };
			case ast::expr::SwizzleKind::e3023:
				return { 3, 0, 2, 3 };
			case ast::expr::SwizzleKind::e3030:
				return { 3, 0, 3, 0 };
			case ast::expr::SwizzleKind::e3031:
				return { 3, 0, 3, 1 };
			case ast::expr::SwizzleKind::e3032:
				return { 3, 0, 3, 2 };
			case ast::expr::SwizzleKind::e3033:
				return { 3, 0, 3, 3 };
			case ast::expr::SwizzleKind::e3100:
				return { 3, 1, 0, 0 };
			case ast::expr::SwizzleKind::e3101:
				return { 3, 1, 0, 1 };
			case ast::expr::SwizzleKind::e3102:
				return { 3, 1, 0, 2 };
			case ast::expr::SwizzleKind::e3103:
				return { 3, 1, 0, 3 };
			case ast::expr::SwizzleKind::e3110:
				return { 3, 1, 1, 0 };
			case ast::expr::SwizzleKind::e3111:
				return { 3, 1, 1, 1 };
			case ast::expr::SwizzleKind::e3112:
				return { 3, 1, 1, 2 };
			case ast::expr::SwizzleKind::e3113:
				return { 3, 1, 1, 3 };
			case ast::expr::SwizzleKind::e3120:
				return { 3, 1, 2, 0 };
			case ast::expr::SwizzleKind::e3121:
				return { 3, 1, 2, 1 };
			case ast::expr::SwizzleKind::e3122:
				return { 3, 1, 2, 2 };
			case ast::expr::SwizzleKind::e3123:
				return { 3, 1, 2, 3 };
			case ast::expr::SwizzleKind::e3130:
				return { 3, 1, 3, 0 };
			case ast::expr::SwizzleKind::e3131:
				return { 3, 1, 3, 1 };
			case ast::expr::SwizzleKind::e3132:
				return { 3, 1, 3, 2 };
			case ast::expr::SwizzleKind::e3133:
				return { 3, 1, 3, 3 };
			case ast::expr::SwizzleKind::e3200:
				return { 3, 2, 0, 0 };
			case ast::expr::SwizzleKind::e3201:
				return { 3, 2, 0, 1 };
			case ast::expr::SwizzleKind::e3202:
				return { 3, 2, 0, 2 };
			case ast::expr::SwizzleKind::e3203:
				return { 3, 2, 0, 3 };
			case ast::expr::SwizzleKind::e3210:
				return { 3, 2, 1, 0 };
			case ast::expr::SwizzleKind::e3211:
				return { 3, 2, 1, 1 };
			case ast::expr::SwizzleKind::e3212:
				return { 3, 2, 1, 2 };
			case ast::expr::SwizzleKind::e3213:
				return { 3, 2, 1, 3 };
			case ast::expr::SwizzleKind::e3220:
				return { 3, 2, 2, 0 };
			case ast::expr::SwizzleKind::e3221:
				return { 3, 2, 2, 1 };
			case ast::expr::SwizzleKind::e3222:
				return { 3, 2, 2, 2 };
			case ast::expr::SwizzleKind::e3223:
				return { 3, 2, 2, 3 };
			case ast::expr::SwizzleKind::e3230:
				return { 3, 2, 3, 0 };
			case ast::expr::SwizzleKind::e3231:
				return { 3, 2, 3, 1 };
			case ast::expr::SwizzleKind::e3232:
				return { 3, 2, 3, 2 };
			case ast::expr::SwizzleKind::e3233:
				return { 3, 2, 3, 3 };
			case ast::expr::SwizzleKind::e3300:
				return { 3, 3, 0, 0 };
			case ast::expr::SwizzleKind::e3301:
				return { 3, 3, 0, 1 };
			case ast::expr::SwizzleKind::e3302:
				return { 3, 3, 0, 2 };
			case ast::expr::SwizzleKind::e3303:
				return { 3, 3, 0, 3 };
			case ast::expr::SwizzleKind::e3310:
				return { 3, 3, 1, 0 };
			case ast::expr::SwizzleKind::e3311:
				return { 3, 3, 1, 1 };
			case ast::expr::SwizzleKind::e3312:
				return { 3, 3, 1, 2 };
			case ast::expr::SwizzleKind::e3313:
				return { 3, 3, 1, 3 };
			case ast::expr::SwizzleKind::e3320:
				return { 3, 3, 2, 0 };
			case ast::expr::SwizzleKind::e3321:
				return { 3, 3, 2, 1 };
			case ast::expr::SwizzleKind::e3322:
				return { 3, 3, 2, 2 };
			case ast::expr::SwizzleKind::e3323:
				return { 3, 3, 2, 3 };
			case ast::expr::SwizzleKind::e3330:
				return { 3, 3, 3, 0 };
			case ast::expr::SwizzleKind::e3331:
				return { 3, 3, 3, 1 };
			case ast::expr::SwizzleKind::e3332:
				return { 3, 3, 3, 2 };
			case ast::expr::SwizzleKind::e3333:
				return { 3, 3, 3, 3 };
			default:
				assert( false && "Unsupported SwizzleKind." );
				return {};
			}
		}

		class AccessChainLineariser
			: public expr::SimpleVisitor
		{
		public:
			static std::vector< expr::Expr * > submit( expr::Expr * expr
				, expr::ExprList & idents )
			{
				std::vector< expr::Expr * > result;
				AccessChainLineariser vis{ result, idents };
				expr->accept( &vis );
				return result;
			}

		private:
			AccessChainLineariser( std::vector< expr::Expr * > & result
				, expr::ExprList & idents )
				: m_result{ result }
				, m_idents{ idents }
			{
			}

			void visitUnaryExpr( expr::Unary * expr )override
			{
				assert( false && "Unexpected expr::Unary ?" );
			}

			void visitBinaryExpr( expr::Binary * expr )override
			{
				assert( false && "Unexpected expr::Binary ?" );
			}

			void visitMbrSelectExpr( expr::MbrSelect * expr )override
			{
				auto outer = submit( expr->getOuterExpr(), m_idents );
				auto inner = submit( expr->getOperand(), m_idents );
				m_result = outer;
				m_result.insert( m_result.end(), inner.begin(), inner.end() );
			}

			void visitArrayAccessExpr( expr::ArrayAccess * expr )override
			{
				auto lhs = submit( expr->getLHS(), m_idents );
				auto rhs = submit( expr->getRHS(), m_idents );
				m_result = lhs;
				m_result.insert( m_result.end(), rhs.begin(), rhs.end() );
			}

			void visitIdentifierExpr( expr::Identifier * expr )override
			{
				if ( expr->getVariable()->isMember()
					&& expr->getType()->isMember() )
				{
					m_idents.emplace_back( makeIdent( expr->getVariable()->getOuter() ) );
					auto ident = m_idents.back().get();
					m_result = submit( ident, m_idents );
				}

				m_result.push_back( expr );
			}

			void visitLiteralExpr( expr::Literal * expr )override
			{
				m_result.push_back( expr );
			}

			void visitSwizzleExpr( expr::Swizzle * expr )override
			{
				m_result.push_back( expr );
			}
			
			void visitAggrInitExpr( expr::AggrInit * )override
			{
				assert( false && "Unexpected expr::AggrInit ?" );
			}

			void visitFnCallExpr( expr::FnCall * )override
			{
				assert( false && "Unexpected expr::FnCall ?" );
			}

			void visitImageAccessCallExpr( expr::ImageAccessCall * )override
			{
				assert( false && "Unexpected expr::ImageAccessCall ?" );
			}

			void visitInitExpr( expr::Init * )override
			{
				assert( false && "Unexpected expr::Init ?" );
			}

			void visitIntrinsicCallExpr( expr::IntrinsicCall * )override
			{
				assert( false && "Unexpected expr::IntrinsicCall ?" );
			}

			void visitQuestionExpr( expr::Question * )override
			{
				assert( false && "Unexpected expr::Question ?" );
			}

			void visitSwitchCaseExpr( expr::SwitchCase * )override
			{
				assert( false && "Unexpected expr::SwitchCase ?" );
			}

			void visitSwitchTestExpr( expr::SwitchTest * )override
			{
				assert( false && "Unexpected expr::SwitchTest ?" );
			}

			void visitTextureAccessCallExpr( expr::TextureAccessCall * )override
			{
				assert( false && "Unexpected expr::TextureAccessCall ?" );
			}

		private:
			std::vector< expr::Expr * > & m_result;
			expr::ExprList & m_idents;
		};

		class AccessChainCreator
			: public expr::SimpleVisitor
		{
		public:
			static IdList submit( std::vector< expr::Expr * > const & exprs
				, Module & module
				, Block & currentBlock )
			{
				IdList result;
				assert( exprs.size() >= 2u );
				result.push_back( submit( exprs[0]
					, module
					, currentBlock ) );

				for ( size_t i = 1u; i < exprs.size(); ++i )
				{
					auto & expr = exprs[i];
					result.push_back( submit( result.back()
						, expr
						, module
						, currentBlock ) );
				}

				return result;
			}

		private:
			AccessChainCreator( spv::Id & result
				, spv::Id parent
				, Module & module
				, Block & currentBlock )
				: m_result{ result }
				, m_module{ module }
				, m_currentBlock{ currentBlock }
				, m_parent{ parent }
			{
			}

			static spv::Id submit( expr::Expr * expr
				, Module & module
				, Block & currentBlock )
			{
				spv::Id result;
				AccessChainCreator vis{ result, 0u, module, currentBlock };
				expr->accept( &vis );
				return result;
			}

			static spv::Id submit( spv::Id parent
				, expr::Expr * expr
				, Module & module
				, Block & currentBlock )
			{
				spv::Id result;
				AccessChainCreator vis{ result, parent, module, currentBlock };
				expr->accept( &vis );
				return result;
			}

			void visitUnaryExpr( expr::Unary * expr )override
			{
				assert( false && "Unexpected expr::Unary ?" );
			}

			void visitBinaryExpr( expr::Binary * expr )override
			{
				assert( false && "Unexpected expr::Binary ?" );
			}

			void visitMbrSelectExpr( expr::MbrSelect * expr )override
			{
				assert( m_parent != 0u );
				m_result = submit( expr->getOperand(), m_module, m_currentBlock );
			}

			void visitArrayAccessExpr( expr::ArrayAccess * expr )override
			{
				assert( m_parent != 0u );
				m_result = submit( expr->getRHS(), m_module, m_currentBlock );
			}

			void visitIdentifierExpr( expr::Identifier * expr )override
			{
				auto var = expr->getVariable();

				if ( m_parent != 0u )
				{
					// Leaf or Intermediate identifier :
					// Store member only (parent has already be added).
					m_result = m_module.registerMemberVariableIndex( expr->getType() );
				}
				else
				{
					// Head identifier
					m_result = m_module.registerVariable( var->getName()
						, getStorageClass( getOutermost( var ) )
						, expr->getType() );
				}
			}

			void visitLiteralExpr( expr::Literal * expr )override
			{
				assert( m_parent != 0u );

				switch ( expr->getLiteralType() )
				{
				case expr::LiteralType::eBool:
					m_result = m_module.registerLiteral( expr->getValue< expr::LiteralType::eBool >() );
					break;
				case expr::LiteralType::eInt:
					m_result = m_module.registerLiteral( expr->getValue< expr::LiteralType::eInt >() );
					break;
				case expr::LiteralType::eUInt:
					m_result = m_module.registerLiteral( expr->getValue< expr::LiteralType::eUInt >() );
					break;
				case expr::LiteralType::eFloat:
					m_result = m_module.registerLiteral( expr->getValue< expr::LiteralType::eFloat >() );
					break;
				case expr::LiteralType::eDouble:
					m_result = m_module.registerLiteral( expr->getValue< expr::LiteralType::eDouble >() );
					break;
				default:
					assert( false && "Unsupported literal type" );
					break;
				}
			}

			void visitSwizzleExpr( expr::Swizzle * expr )override
			{
				assert( m_parent != 0u );

				auto rawType = m_module.registerType( expr->getType() );
				auto pointerType = m_module.registerPointerType( rawType, spv::StorageClass::Function );
				auto intermediate = m_module.getIntermediateResult();
				m_currentBlock.instructions.emplace_back( makeVectorShuffle( intermediate
					, pointerType
					, m_parent
					, getSwizzleComponents( expr->getSwizzle() ) ) );
				m_result = m_module.getIntermediateResult();
				m_currentBlock.instructions.emplace_back( makeLoad( m_result, rawType, intermediate ) );
				m_module.putIntermediateResult( intermediate );
			}

			void visitAggrInitExpr( expr::AggrInit * )override
			{
				assert( false && "Unexpected expr::AggrInit ?" );
			}

			void visitFnCallExpr( expr::FnCall * )override
			{
				assert( false && "Unexpected expr::FnCall ?" );
			}

			void visitImageAccessCallExpr( expr::ImageAccessCall * )override
			{
				assert( false && "Unexpected expr::ImageAccessCall ?" );
			}

			void visitInitExpr( expr::Init * )override
			{
				assert( false && "Unexpected expr::Init ?" );
			}

			void visitIntrinsicCallExpr( expr::IntrinsicCall * )override
			{
				assert( false && "Unexpected expr::IntrinsicCall ?" );
			}

			void visitQuestionExpr( expr::Question * )override
			{
				assert( false && "Unexpected expr::Question ?" );
			}

			void visitSwitchCaseExpr( expr::SwitchCase * )override
			{
				assert( false && "Unexpected expr::SwitchCase ?" );
			}

			void visitSwitchTestExpr( expr::SwitchTest * )override
			{
				assert( false && "Unexpected expr::SwitchTest ?" );
			}

			void visitTextureAccessCallExpr( expr::TextureAccessCall * )override
			{
				assert( false && "Unexpected expr::TextureAccessCall ?" );
			}

		private:
			spv::Id & m_result;
			Module & m_module;
			Block & m_currentBlock;
			spv::Id m_parent;
		};
	}

	spv::StorageClass getStorageClass( var::VariablePtr var )
	{
		spv::StorageClass result = spv::StorageClass::Function;

		if ( var->isBound() )
		{
			result = spv::StorageClass::Uniform;
		}
		else if ( var->isImage() )
		{
			result = spv::StorageClass::Image;
		}
		else if ( var->isSampler() )
		{
			result = spv::StorageClass::Uniform;
		}
		else if ( var->isShaderInput() )
		{
			result = spv::StorageClass::Input;
		}
		else if ( var->isShaderOutput() )
		{
			result = spv::StorageClass::Output;
		}
		else if ( var->isShaderConstant() )
		{
			result = spv::StorageClass::PushConstant;
		}

		return result;
	}

	spv::Id ExprVisitor::submit( expr::Expr * expr
		, Block & currentBlock
		, Module & module
		, LinkedVars const & linkedVars )
	{
		spv::Id result;
		ExprVisitor vis{ result
			, currentBlock
			, module
			, linkedVars };
		expr->accept( &vis );
		return result;
	}

	ExprVisitor::ExprVisitor( spv::Id & result
		, Block & currentBlock
		, Module & module
		, LinkedVars const & linkedVars )
		: m_result{ result }
		, m_currentBlock{ currentBlock }
		, m_module{ module }
		, m_linkedVars{ linkedVars }
	{
	}

	void ExprVisitor::visitAssignmentExpr( expr::Assign * expr )
	{
		auto lhs = submit( expr->getLHS(), m_currentBlock, m_module, m_linkedVars );
		auto rhs = submit( expr->getRHS(), m_currentBlock, m_module, m_linkedVars );
		auto type = m_module.registerType( expr->getType() );
		m_currentBlock.instructions.emplace_back( makeStore( lhs, rhs ) );
		m_result = lhs;
	}

	void ExprVisitor::visitOpAssignmentExpr( expr::Assign * expr )
	{
		auto typeId = m_module.registerType( expr->getType() );
		auto lhsId = submit( expr->getLHS(), m_currentBlock, m_module, m_linkedVars );
		auto loadedLhsId = m_module.loadVariable( lhsId
			, expr->getType()
			, m_currentBlock );
		auto rhsId = submit( expr->getRHS(), m_currentBlock, m_module, m_linkedVars );

		auto intermediateId = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeBinInstruction( expr->getKind()
			, expr->getLHS()->getType()->getKind()
			, expr->getRHS()->getType()->getKind()
			, intermediateId
			, typeId
			, loadedLhsId
			, rhsId ) );
		m_currentBlock.instructions.emplace_back( makeStore( lhsId, intermediateId ) );
		m_module.putIntermediateResult( intermediateId );
		m_module.putIntermediateResult( loadedLhsId );
		m_result = lhsId;
	}

	void ExprVisitor::visitUnaryExpr( expr::Unary * expr )
	{
		auto operand = submit( expr->getOperand(), m_currentBlock, m_module, m_linkedVars );
		auto type = m_module.registerType( expr->getType() );
		auto intermediate = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeUnInstruction( expr->getKind()
			, expr->getType()->getKind()
			, intermediate
			, type
			, operand ) );
		m_result = intermediate;
	}

	void ExprVisitor::visitBinaryExpr( expr::Binary * expr )
	{
		auto lhs = submit( expr->getLHS(), m_currentBlock, m_module, m_linkedVars );
		auto rhs = submit( expr->getRHS(), m_currentBlock, m_module, m_linkedVars );
		auto type = m_module.registerType( expr->getType() );
		auto intermediate = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeBinInstruction( expr->getKind()
			, expr->getLHS()->getType()->getKind()
			, expr->getRHS()->getType()->getKind()
			, intermediate
			, type
			, lhs
			, rhs ) );
		m_result = intermediate;
	}

	void ExprVisitor::visitPreDecrementExpr( expr::PreDecrement * expr )
	{
		auto literal = m_module.registerLiteral( 1 );
		auto operand = submit( expr->getOperand(), m_currentBlock, m_module, m_linkedVars );
		auto type = m_module.registerType( expr->getType() );
		auto intermediate = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeBinInstruction( expr::Kind::eMinus
			, expr->getOperand()->getType()->getKind()
			, type::Kind::eInt
			, intermediate
			, operand
			, literal ) );
		m_currentBlock.instructions.emplace_back( makeStore( operand, intermediate ) );
		m_result = intermediate;
	}

	void ExprVisitor::visitPreIncrementExpr( expr::PreIncrement * expr )
	{
		auto literal = m_module.registerLiteral( 1 );
		auto operand = submit( expr->getOperand(), m_currentBlock, m_module, m_linkedVars );
		auto type = m_module.registerType( expr->getType() );
		auto intermediate = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeBinInstruction( expr::Kind::eAdd
			, expr->getOperand()->getType()->getKind()
			, type::Kind::eInt
			, intermediate
			, operand
			, literal ) );
		operand = m_module.getNonIntermediate( operand );
		m_currentBlock.instructions.emplace_back( makeStore( operand, intermediate ) );
		m_result = intermediate;
	}

	void ExprVisitor::visitPostDecrementExpr( expr::PostDecrement * expr )
	{
		auto literal1 = m_module.registerLiteral( 1 );
		auto literal0 = m_module.registerLiteral( 0 );
		auto operand = submit( expr->getOperand(), m_currentBlock, m_module, m_linkedVars );
		auto type = m_module.registerType( expr->getType() );
		auto intermediate = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeBinInstruction( expr::Kind::eAdd
			, expr->getOperand()->getType()->getKind()
			, type::Kind::eInt
			, intermediate
			, operand
			, literal0 ) );
		m_currentBlock.instructions.emplace_back( makeBinInstruction( expr::Kind::eMinus
			, expr->getOperand()->getType()->getKind()
			, type::Kind::eInt
			, operand
			, operand
			, literal1 ) );
		m_result = intermediate;
	}

	void ExprVisitor::visitPostIncrementExpr( expr::PostIncrement * expr )
	{
		auto literal1 = m_module.registerLiteral( 1 );
		auto literal0 = m_module.registerLiteral( 0 );
		auto operand = submit( expr->getOperand(), m_currentBlock, m_module, m_linkedVars );
		auto type = m_module.registerType( expr->getType() );
		auto intermediate = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeBinInstruction( expr::Kind::eAdd
			, expr->getOperand()->getType()->getKind()
			, type::Kind::eInt
			, intermediate
			, operand
			, literal0 ) );
		m_currentBlock.instructions.emplace_back( makeBinInstruction( expr::Kind::eAdd
			, expr->getOperand()->getType()->getKind()
			, type::Kind::eInt
			, operand
			, operand
			, literal1 ) );
		m_result = intermediate;
	}

	void ExprVisitor::visitUnaryPlusExpr( expr::UnaryPlus * expr )
	{
		m_result = submit( expr, m_currentBlock, m_module, m_linkedVars );
	}

	void ExprVisitor::visitAddAssignExpr( expr::AddAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitAndAssignExpr( expr::AndAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitAssignExpr( expr::Assign * expr )
	{
		visitAssignmentExpr( expr );
	}

	void ExprVisitor::visitDivideAssignExpr( expr::DivideAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitLShiftAssignExpr( expr::LShiftAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitMinusAssignExpr( expr::MinusAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitModuloAssignExpr( expr::ModuloAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitOrAssignExpr( expr::OrAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitRShiftAssignExpr( expr::RShiftAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitTimesAssignExpr( expr::TimesAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitXorAssignExpr( expr::XorAssign * expr )
	{
		visitOpAssignmentExpr( expr );
	}

	void ExprVisitor::visitAggrInitExpr( expr::AggrInit * expr )
	{
		auto typeId = m_module.registerType( expr->getType() );
		IdList initialisers;
		bool allLiterals = true;

		for ( auto & init : expr->getInitialisers() )
		{
			auto initialiser = submit( init.get(), m_currentBlock, m_module, m_linkedVars );
			initialisers.push_back( initialiser );
			allLiterals = allLiterals
				&& ( init->getKind() == expr::Kind::eLiteral );
		}

		spv::Id init;

		if ( allLiterals )
		{
			init = m_module.registerLiteral( initialisers, expr->getType() );
		}
		else
		{
			init = m_module.getIntermediateResult();
			m_currentBlock.instructions.emplace_back( makeInstruction( spv::Op::OpCompositeConstruct
				, init
				, typeId
				, { initialisers } ) );
		}

		m_result = submit( expr->getIdentifier(), m_currentBlock, m_module, m_linkedVars );
		m_currentBlock.instructions.emplace_back( makeStore( m_result, init ) );
		m_module.putIntermediateResult( init );
	}

	void ExprVisitor::makeAccessChain( expr::Expr * expr )
	{
		// Create Access Chain.
		expr::ExprList idents;
		auto accessChainExprs = AccessChainLineariser::submit( expr, idents );
		auto accessChain = AccessChainCreator::submit( accessChainExprs
			, m_module
			, m_currentBlock );
		// Reserve the ID for the result.
		auto intermediate = m_module.getIntermediateResult();
		// Register the type pointed to.
		auto rawType = m_module.registerType( expr->getType() );
		// Register the pointer to the type.
		auto pointerType = m_module.registerPointerType( rawType
			, getStorageClass( getOutermost( findIdentifier( expr )->getVariable() ) ) );
		// Write access chain => resultId = pointerType( outer.members + index ).
		m_currentBlock.instructions.emplace_back( spirv::makeAccessChain( spv::Op::OpAccessChain
			, intermediate
			, pointerType
			, accessChain ) );
		m_result = m_module.loadVariable( intermediate
			, expr->getType()
			, m_currentBlock );
		m_module.putIntermediateResult( intermediate );
	}

	void ExprVisitor::visitArrayAccessExpr( expr::ArrayAccess * expr )
	{
		makeAccessChain( expr );
	}

	void ExprVisitor::visitMbrSelectExpr( expr::MbrSelect * expr )
	{
		makeAccessChain( expr );
	}

	void ExprVisitor::visitFnCallExpr( expr::FnCall * expr )
	{
		IdList params;
		bool allLiterals = true;

		for ( auto & arg : expr->getArgList() )
		{
			auto id = submit( arg.get(), m_currentBlock, m_module, m_linkedVars );
			params.push_back( id );
			allLiterals = allLiterals
				&& ( arg->getKind() == expr::Kind::eLiteral );
		}

		auto type = m_module.registerType( expr->getType() );
		spv::Id result;

		if ( !isCtor( expr->getFn()->getVariable()->getName() ) )
		{
			result = m_module.getIntermediateResult();
			auto fnId = submit( expr->getFn(), m_currentBlock, m_module, m_linkedVars );
			m_currentBlock.instructions.emplace_back( makeInstruction( expr->getKind()
				, expr->getType()->getKind()
				, result
				, type
				, params ) );
		}
		else if ( allLiterals )
		{
			result = m_module.registerLiteral( params, expr->getType() );
		}
		else
		{
			result = m_module.getIntermediateResult();
			m_currentBlock.instructions.emplace_back( makeInstruction( spv::Op::OpCompositeConstruct
				, result
				, type
				, params ) );
		}

		m_result = result;
	}

	void ExprVisitor::visitIdentifierExpr( expr::Identifier * expr )
	{
		auto var = expr->getVariable();

		if ( var->isMember() )
		{
			makeAccessChain( expr );
		}
		else
		{
			m_result = m_module.registerVariable( var->getName()
				, getStorageClass( var )
				, expr->getType() );

			if ( var->isLocale()
				|| var->isShaderInput() )
			{
				m_result = m_module.loadVariable( m_result
					, expr->getType()
					, m_currentBlock );
			}
		}
	}

	void ExprVisitor::visitImageAccessCallExpr( expr::ImageAccessCall * expr )
	{
		IdList params;

		for ( auto & arg : expr->getArgList() )
		{
			auto id = submit( arg.get(), m_currentBlock, m_module, m_linkedVars );
			params.push_back( id );
		}

		auto type = m_module.registerType( expr->getType() );
		auto intermediate = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeInstruction( expr->getImageAccess()
			, intermediate
			, type
			, params ) );
		m_result = intermediate;
	}

	void ExprVisitor::visitInitExpr( expr::Init * expr )
	{
		auto result = m_module.registerVariable( expr->getIdentifier()->getVariable()->getName()
			, ( ( expr->getIdentifier()->getVariable()->isLocale() || expr->getIdentifier()->getVariable()->isInputParam() )
				? spv::StorageClass::Function
				: spv::StorageClass::UniformConstant )
			, expr->getType() );
		auto type = m_module.registerType( expr->getType() );
		auto init = submit( expr->getInitialiser(), m_currentBlock, m_module, m_linkedVars );
		m_currentBlock.instructions.emplace_back( makeStore( result, init ) );
		m_result = result;
	}

	void ExprVisitor::visitIntrinsicCallExpr( expr::IntrinsicCall * expr )
	{
		IdList params;

		for ( auto & arg : expr->getArgList() )
		{
			auto id = submit( arg.get(), m_currentBlock, m_module, m_linkedVars );
			params.push_back( id );
		}

		auto type = m_module.registerType( expr->getType() );
		auto intermediate = m_module.getIntermediateResult();
		bool isExtension;
		auto opCode = getSpirVName( expr->getIntrinsic(), isExtension );

		if ( isExtension )
		{
			params.insert( params.begin(), opCode );
			m_currentBlock.instructions.emplace_back( makeInstruction( spv::Op::OpExtInst
				, intermediate
				, type
				, params ) );
		}
		else
		{
			m_currentBlock.instructions.emplace_back( makeInstruction( spv::Op( opCode )
				, intermediate
				, type
				, params ) );
		}

		m_result = intermediate;
	}

	void ExprVisitor::visitLiteralExpr( expr::Literal * expr )
	{
		switch ( expr->getLiteralType() )
		{
		case expr::LiteralType::eBool:
			m_result = m_module.registerLiteral( expr->getValue< expr::LiteralType::eBool >() );
			break;
		case expr::LiteralType::eInt:
			m_result = m_module.registerLiteral( expr->getValue< expr::LiteralType::eInt >() );
			break;
		case expr::LiteralType::eUInt:
			m_result = m_module.registerLiteral( expr->getValue< expr::LiteralType::eUInt >() );
			break;
		case expr::LiteralType::eFloat:
			m_result = m_module.registerLiteral( expr->getValue< expr::LiteralType::eFloat >() );
			break;
		case expr::LiteralType::eDouble:
			m_result = m_module.registerLiteral( expr->getValue< expr::LiteralType::eDouble >() );
			break;
		default:
			assert( false && "Unsupported literal type" );
			break;
		}
	}

	void ExprVisitor::visitQuestionExpr( expr::Question * expr )
	{
		auto ctrlId = submit( expr->getCtrlExpr(), m_currentBlock, m_module, m_linkedVars );
		auto trueId = submit( expr->getTrueExpr(), m_currentBlock, m_module, m_linkedVars );
		auto falseId = submit( expr->getFalseExpr(), m_currentBlock, m_module, m_linkedVars );
		auto type = m_module.registerType( expr->getType() );
		auto intermediate = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeInstruction( expr->getKind()
			, expr->getType()->getKind()
			, intermediate
			, type
			, IdList{ ctrlId, trueId, falseId } ) );
		m_result = intermediate;
	}

	void ExprVisitor::visitSwitchCaseExpr( expr::SwitchCase * expr )
	{
		m_result = submit( expr->getLabel(), m_currentBlock, m_module, m_linkedVars );
	}

	void ExprVisitor::visitSwitchTestExpr( expr::SwitchTest * expr )
	{
		m_result = submit( expr->getValue(), m_currentBlock, m_module, m_linkedVars );
	}

	void ExprVisitor::visitSwizzleExpr( expr::Swizzle * expr )
	{
		auto outer = submit( expr->getOuterExpr(), m_currentBlock, m_module, m_linkedVars );
		auto rawType = m_module.registerType( expr->getType() );
		auto pointerType = m_module.registerPointerType( rawType, spv::StorageClass::Function );
		auto intermediate = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeVectorShuffle( intermediate
			, pointerType
			, outer
			, getSwizzleComponents( expr->getSwizzle() ) ) );
		m_result = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeStore( m_result, intermediate ) );
		m_module.putIntermediateResult( intermediate );
	}

	void ExprVisitor::visitTextureAccessCallExpr( expr::TextureAccessCall * expr )
	{
		IdList args;

		for ( auto & arg : expr->getArgList() )
		{
			if ( arg->getKind() == expr::Kind::eIdentifier )
			{
				auto ident = findIdentifier( arg );
				auto it = m_linkedVars.find( ident->getVariable() );

				if ( m_linkedVars.end() != it )
				{
					args.emplace_back( submit( makeIdent( it->second.sampledImage ).get(), m_currentBlock, m_module, m_linkedVars ) );
				}
				else
				{
					args.emplace_back( submit( arg.get(), m_currentBlock, m_module, m_linkedVars ) );
				}
			}
			else
			{
				args.emplace_back( submit( arg.get(), m_currentBlock, m_module, m_linkedVars ) );
			}
		}

		auto type = m_module.registerType( expr->getType() );
		auto intermediate = m_module.getIntermediateResult();
		m_currentBlock.instructions.emplace_back( makeInstruction( expr->getTextureAccess()
			, intermediate
			, type
			, args ) );
		m_result = intermediate;
	}
}