// =============================================================================
//  ColorMap.hpp
//
//  Written in 2014 by Dairoku Sekiguchi (sekiguchi at acm dot org)
//
//  To the extent possible under law, the author(s) have dedicated all copyright
//  and related and neighboring rights to this software to the public domain worldwide.
//  This software is distributed without any warranty.
//
//  You should have received a copy of the CC0 Public Domain Dedication along with
//  this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
// =============================================================================
/*!
	\file		viw/utils/ColorMap.h
	\author		Dairoku Sekiguchi
	\version	1.0.0
	\date		2015/10/04
	\brief		Header file for viw library types

	This file defines types for viw library
*/

#ifndef VIW_UTIL_COLORMAP_H
#define VIW_UTIL_COLORMAP_H

// Includes --------------------------------------------------------------------
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "viw/Exception.hpp"

// Macros ----------------------------------------------------------------------
//#define	VIW_COLORMAP_USE_D50


// Namespace -------------------------------------------------------------------
namespace viw
{
 namespace utils
 {
	// -------------------------------------------------------------------------
	// ColorMap class
	// -------------------------------------------------------------------------
	class	ColorMap
	{
	public:
		// Constatns -----------------------------------------------------------
		enum ColorMapIndex
		{
			// Linear
			CMIndex_GrayScale		= 1,
			CMIndex_Jet,
			CMIndex_Rainbow,
			CMIndex_RainbowWide,
			CMIndex_Spectrum,
			CMIndex_SpectrumWide,
			CMIndex_Thermal,
			CMIndex_ThermalWide,
			
			// Diverging
			CMIndex_CoolWarm,
			CMIndex_PurpleOrange,
			CMIndex_GreenPurple,
			CMIndex_BlueDarkYellow,
			CMIndex_GreenRed,
			
			// For Internal use only
			CMIndex_End			= -1
		};
		enum ColorMapType
		{
			CMType_Linear		= 1,
			CMType_Diverging
		};

		// Static Functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// getColorMap
		// ---------------------------------------------------------------------
		static void	getColorMap(ColorMapIndex inIndex, int inColorNum, unsigned char *outColorMap)
		{
			int	i, num, total;
			const ColorMapData		*colorMapData;
			double	prevRatio;
			const unsigned char		*rgb1, *rgb2;
			
			colorMapData = getColorMapData(inIndex);
			if (colorMapData == NULL)
				return;
			
			total = 0;
			if (colorMapData[0].ratio != 0)
			{
				num = (int )((double )inColorNum * colorMapData[0].ratio);
				for (i = 0; i < num; i++)
				{
					outColorMap[0] = colorMapData[0].rgb.R;
					outColorMap[1] = colorMapData[0].rgb.G;
					outColorMap[2] = colorMapData[0].rgb.B;
					outColorMap += 3;
					total++;
				}
			}

			while (colorMapData[1].index == inIndex)
			{
				rgb1 = (const unsigned char *)&(colorMapData[0].rgb);
				rgb2 = (const unsigned char *)&(colorMapData[1].rgb);
				if (colorMapData[1].ratio == 1.0)
				{
					num = inColorNum - total;
				}
				else
				{
					num = (int)((double)inColorNum *
						(colorMapData[1].ratio - colorMapData[0].ratio));
				}

				switch (colorMapData[0].type)
				{
					case CMType_Linear:
						calcLinearColorMap(rgb1, rgb2, num, outColorMap);
						break;
					case CMType_Diverging:
						calcDivergingColorMap(rgb1, rgb2, num, outColorMap);
						break;
				}
				
				outColorMap += num * 3;
				total += num;
				colorMapData++;
			}

			if (colorMapData[0].ratio != 1.0)
			{
				num = (int )((double )inColorNum * (1.0 - colorMapData[0].ratio));
				for (i = 0; i < num; i++)
				{
					outColorMap[0] = colorMapData[0].rgb.R;
					outColorMap[1] = colorMapData[0].rgb.G;
					outColorMap[2] = colorMapData[0].rgb.B;
					outColorMap += 3;
				}
			}
		}
		// ---------------------------------------------------------------------
		// getMultiColorMap
		// ---------------------------------------------------------------------
		static void	getMultiColorMap(ColorMapIndex inIndex, int inColorNum, int inMultiNum, unsigned char *outColorMap)
		{
			int	unit = inColorNum / inMultiNum;
			
			inMultiNum--;
			for (int i = 0; i < inMultiNum; i++)
			{
				getColorMap(inIndex, unit, outColorMap);
				outColorMap += (unit * 3);
				inColorNum -= unit;
			}
			getColorMap(inIndex, inColorNum, outColorMap);
		}
		// ---------------------------------------------------------------------
		// calcLinearColorMap
		// ---------------------------------------------------------------------
		static void	calcLinearColorMap(const unsigned char *inRgb1, const unsigned char *inRgb2,
										 int inColorNum, unsigned char *outColorMap)
		{
			double	interp, k, v;
			
			k = 1.0 / (double )(inColorNum - 1.0);
			for (int i = 0; i < inColorNum; i++)
			{
				interp = (double )i * k;
				for (int j = 0; j < 3; j++)
				{
					v = (1.0 - interp) * inRgb1[j] + interp * inRgb2[j];
					if (v > 255)
						v = 255;
					outColorMap[i * 3 + j] = (unsigned char)v;
				}
			}
		}
		// ---------------------------------------------------------------------
		// calcDivergingColorMap
		// ---------------------------------------------------------------------
		static void	calcDivergingColorMap(const unsigned char *inRgb1, const unsigned char *inRgb2,
										 int inColorNum, unsigned char *outColorMap)
		{
			double	interp, k;
			
			k = 1.0 / (double )(inColorNum - 1.0);
			for (int i = 0; i < inColorNum; i++)
			{
				interp = (double )i * k;
				interpolateColor(inRgb1, inRgb2, interp, &(outColorMap[i * 3]));
			}
		}
		// ---------------------------------------------------------------------
		// interpolateColor
		// ---------------------------------------------------------------------
		static void	interpolateColor(const unsigned char *inRgb1, const unsigned char *inRgb2,
										 double inInterp, unsigned char *outRgb)
		{
			double	msh1[3], msh2[3], msh[3], m;
			
			convRgbToMsh(inRgb1, msh1);
			convRgbToMsh(inRgb2, msh2);
			
			if ((msh1[1] > 0.05 && msh2[1] > 0.05) && fabs(msh1[2] - msh2[2]) > 1.0472)
			{
				if (msh1[0] > msh2[0])
					m = msh1[0];
				else
					m = msh2[0];
				if (m < 88)
					m = 88;
				if (inInterp < 0.5)
				{
					msh2[0] = m;
					msh2[1] = 0;
					msh2[2] = 0;
					inInterp = 2 * inInterp;
				}
				else
				{
					msh1[0] = m;
					msh1[1] = 0;
					msh1[2] = 0;
					inInterp = 2 * inInterp - 1;
				}
			}
			
			if (msh1[1] < 0.05 && msh2[1] > 0.05)
				msh1[2] =  adjustHue(msh2, msh1[0]);
			else if (msh1[1] > 0.05 && msh2[1] < 0.05)
				msh2[2] =  adjustHue(msh1, msh2[0]);
			
			for (int i = 0; i < 3; i++)
				msh[i] = (1 - inInterp) * msh1[i] + inInterp * msh2[i];
			
			convMshToRgb(msh, outRgb);
		}
		// ---------------------------------------------------------------------
		// adjustHue
		// ---------------------------------------------------------------------
		static double	adjustHue(const double *inMsh, double inMunsat)
		{
			if (inMsh[0] >= inMunsat)
				return inMsh[2];
			
			double	hSpin = inMsh[1] * sqrt(inMunsat * inMunsat - inMsh[0] * inMsh[0]) /
									(inMsh[0] * sin(inMsh[1]));

			if (inMsh[2] > -1.0472)
				return inMsh[2] + hSpin;
			return inMsh[2] - hSpin;
		}
		// ---------------------------------------------------------------------
		// convRgbToMsh
		// ---------------------------------------------------------------------
		static void	convRgbToMsh(const unsigned char *inRgb, double *outMsh)
		{
			double	rgbL[3];
			double	xyz[3];
			double	lab[3];
			
			convRgbToLinRGB(inRgb, rgbL);
		#ifdef VIW_COLORMAP_USE_D50
			convLinRgbToXyzD50(rgbL, xyz);
			convXyzD50ToLab(xyz, lab);
		#else
			convLinRgbToXyz(rgbL, xyz);
			convXyzD65ToLab(xyz, lab);
		#endif
			convLabToMsh(lab, outMsh);
		}
		// ---------------------------------------------------------------------
		// convMshToRgb
		// ---------------------------------------------------------------------
		static void	convMshToRgb(const double *inMsh, unsigned char *outRgb)
		{
			double	lab[3];
			double	xyz[3];
			double	rgbL[3];
			
			convMshToLab(inMsh, lab);
		#ifdef VIW_COLORMAP_USE_D50
			convLabToXyzD50(lab, xyz);
			convXyzD50ToLinRgb(xyz, rgbL);
		#else
			convLabToXyzD65(lab, xyz);
			convXyzToLinRgb(xyz, rgbL);
		#endif
			convLinRgbToRGB(rgbL, outRgb);
		}
		// ---------------------------------------------------------------------
		// convLabToMsh
		// ---------------------------------------------------------------------
		static void	convLabToMsh(const double *inLab, double *outMsh)
		{
			outMsh[0] = sqrt(inLab[0] * inLab[0] + inLab[1] * inLab[1] + inLab[2] * inLab[2]);
			outMsh[1] = acos(inLab[0] / outMsh[0]);
			outMsh[2] = atan2(inLab[2], inLab[1]);
		}
		// ---------------------------------------------------------------------
		// convMshToLab
		// ---------------------------------------------------------------------
		static void	convMshToLab(const double *inMsh, double *outLab)
		{
			outLab[0] = inMsh[0] * cos(inMsh[1]);
			outLab[1] = inMsh[0] * sin(inMsh[1]) * cos(inMsh[2]);
			outLab[2] = inMsh[0] * sin(inMsh[1]) * sin(inMsh[2]);
		}
		// ---------------------------------------------------------------------
		// convXyzD50ToLab
		// ---------------------------------------------------------------------
		static void	convXyzD50ToLab(const double *inXyz, double *outLab)
		{
			const double	*wpXyz = getD50WhitePointInXyz();
			
			outLab[0] = 116 *  labSubFunc(inXyz[1] / wpXyz[1]) - 16.0;
			outLab[1] = 500 * (labSubFunc(inXyz[0] / wpXyz[0]) - labSubFunc(inXyz[1] / wpXyz[1]));
			outLab[2] = 200 * (labSubFunc(inXyz[1] / wpXyz[1]) - labSubFunc(inXyz[2] / wpXyz[2]));
		}
		// ---------------------------------------------------------------------
		// convLabToXyzD50
		// ---------------------------------------------------------------------
		static void	convLabToXyzD50(const double *inLab, double *outXyz)
		{
			const double	*wpXyz = getD50WhitePointInXyz();
			
			outXyz[0] = labSubInvFunc((inLab[0] + 16) / 116.0 + (inLab[1] / 500.0)) * wpXyz[0];
			outXyz[1] = labSubInvFunc((inLab[0] + 16) / 116.0) * wpXyz[1];
			outXyz[2] = labSubInvFunc((inLab[0] + 16) / 116.0 - (inLab[2] / 200.0)) * wpXyz[2];
		}
		// ---------------------------------------------------------------------
		// convXyzD65ToLab
		// ---------------------------------------------------------------------
		static void	convXyzD65ToLab(const double *inXyz, double *outLab)
		{
			const double	*wpXyz = getD65WhitePointInXyz();
			
			outLab[0] = 116 *  labSubFunc(inXyz[1] / wpXyz[1]) - 16.0;
			outLab[1] = 500 * (labSubFunc(inXyz[0] / wpXyz[0]) - labSubFunc(inXyz[1] / wpXyz[1]));
			outLab[2] = 200 * (labSubFunc(inXyz[1] / wpXyz[1]) - labSubFunc(inXyz[2] / wpXyz[2]));
		}
		// ---------------------------------------------------------------------
		// convLabToXyzD65
		// ---------------------------------------------------------------------
		static void	convLabToXyzD65(const double *inLab, double *outXyz)
		{
			const double	*wpXyz = getD65WhitePointInXyz();
			
			outXyz[0] = labSubInvFunc((inLab[0] + 16) / 116.0 + (inLab[1] / 500.0)) * wpXyz[0];
			outXyz[1] = labSubInvFunc((inLab[0] + 16) / 116.0) * wpXyz[1];
			outXyz[2] = labSubInvFunc((inLab[0] + 16) / 116.0 - (inLab[2] / 200.0)) * wpXyz[2];
		}
		// ---------------------------------------------------------------------
		// convLinRgbToXyz (from sRGB linear (D65) to XYZ (D65) color space)
		// ---------------------------------------------------------------------
		static void	convLinRgbToXyz(const double *inRgbL, double *outXyz)
		{
			outXyz[0] = 0.412391 * inRgbL[0] + 0.357584 * inRgbL[1] +  0.180481 * inRgbL[2];
			outXyz[1] = 0.212639 * inRgbL[0] + 0.715169 * inRgbL[1] +  0.072192 * inRgbL[2];
			outXyz[2] = 0.019331 * inRgbL[0] + 0.119195 * inRgbL[1] +  0.950532 * inRgbL[2];
		}
		// ---------------------------------------------------------------------
		// convXyzToLinRgb (from XYZ (D65) to sRGB linear (D65) color space)
		// ---------------------------------------------------------------------
		static void	convXyzToLinRgb(const double *inXyz, double *outRgbL)
		{
			outRgbL[0] =  3.240970 * inXyz[0] - 1.537383 * inXyz[1] - 0.498611 * inXyz[2];
			outRgbL[1] = -0.969244 * inXyz[0] + 1.875968 * inXyz[1] + 0.041555 * inXyz[2];
			outRgbL[2] =  0.055630 * inXyz[0] - 0.203977 * inXyz[1] + 1.056972 * inXyz[2];
		}
		// ---------------------------------------------------------------------
		// convLinRgbToXyzD50 (from sRGB linear (D65) to XYZ (D50) color space)
		// ---------------------------------------------------------------------
		static void	convLinRgbToXyzD50(const double *inRgbL, double *outXyz)
		{
			outXyz[0] = 0.436041 * inRgbL[0] +  0.385113 * inRgbL[1] +  0.143046 * inRgbL[2];
			outXyz[1] = 0.222485 * inRgbL[0] +  0.716905 * inRgbL[1] +  0.060610 * inRgbL[2];
			outXyz[2] = 0.013920 * inRgbL[0] +  0.097067 * inRgbL[1] +  0.713913 * inRgbL[2];
		}
		// ---------------------------------------------------------------------
		// convXyzD50ToLinRgb (from XYZ (D50) to sRGB linear (D65) color space)
		// ---------------------------------------------------------------------
		static void	convXyzD50ToLinRgb(const double *inXyz, double *outRgbL)
		{
			outRgbL[0] =  3.134187 * inXyz[0] - 1.617209 * inXyz[1] - 0.490694 * inXyz[2];
			outRgbL[1] = -0.978749 * inXyz[0] + 1.916130 * inXyz[1] + 0.033433 * inXyz[2];
			outRgbL[2] =  0.071964 * inXyz[0] - 0.228994 * inXyz[1] + 1.405754 * inXyz[2];
		}
		// ---------------------------------------------------------------------
		// convRgbToLinRGB (from sRGB to linear sRGB)
		// ---------------------------------------------------------------------
		static void	convRgbToLinRGB(const unsigned char *inRgb, double *outRgbL)
		{
			double	value;
			
			for (int i = 0; i < 3; i++)
			{
				value = (double )inRgb[i] / 255.0;
				if (value <= 0.040450)
					value = value / 12.92;
				else
					value = pow((value + 0.055) / 1.055, 2.4);
				outRgbL[i] = value;
			}
		}
		// ---------------------------------------------------------------------
		// convLinRgbToRGB (from linear sRGB to sRGB)
		// ---------------------------------------------------------------------
		static void	convLinRgbToRGB(const double *inRgbL, unsigned char *outRgb)
		{
			double	value;
			
			for (int i = 0; i < 3; i++)
			{
				value = inRgbL[i];
				if (value <= 0.0031308)
					value =  value * 12.92;
				else
					value = 1.055 * pow(value, 1.0 / 2.4) - 0.055;
				
				value = value * 255;
				if (value < 0)
					value = 0;
				if (value > 255)
					value = 255;
				outRgb[i] = (unsigned char)value;
			}
		}

	private:
		// Typedefs ------------------------------------------------------------
		typedef struct
		{
			unsigned char	R;
			unsigned char	G;
			unsigned char	B;
		} ColorMapRGB;
		typedef struct
		{
			ColorMapIndex	index;
			double			ratio;
			ColorMapType	type;
			ColorMapRGB		rgb;
		} ColorMapData;
		
		// Static Functions ----------------------------------------------------
		// ---------------------------------------------------------------------
		// getD50WhitePointInXyz
		// ---------------------------------------------------------------------
		static const double	*getD50WhitePointInXyz()
		{
			static const double	sD50WhitePoint[3] = {0.9642, 1.0, 0.8249};
			return sD50WhitePoint;
		}
		// ---------------------------------------------------------------------
		// getD50WhitePointInXyz
		// ---------------------------------------------------------------------
		static const double	*getD65WhitePointInXyz()
		{
			static const double	sD65WhitePoint[3] = {0.95047, 1.0, 1.08883};
			return sD65WhitePoint;
		}
		// ---------------------------------------------------------------------
		// labSubFunc
		// ---------------------------------------------------------------------
		static double	labSubFunc(double inT)
		{
			if (inT > 0.008856)
				return pow(inT, (1.0/3.0));
			return 7.78703 * inT + 16.0 / 116.0;
		}
		// ---------------------------------------------------------------------
		// labSubInvFunc
		// ---------------------------------------------------------------------
		static double	labSubInvFunc(double inT)
		{
			if (inT > 0.20689)
				return pow(inT, 3);
			return (inT - 16.0 / 116.0) / 7.78703;
		}
		// ---------------------------------------------------------------------
		// getColorMapData
		// ---------------------------------------------------------------------
		static const ColorMapData	*getColorMapData(ColorMapIndex inIndex)
		{
			static const ColorMapData		sColorMapData[] =
			{
				// GrayScale
				{CMIndex_GrayScale, 0.0, CMType_Linear, {0, 0, 0}},
				{CMIndex_GrayScale, 1.0, CMType_Linear, {255, 255, 255}},
				// Jet
				{ CMIndex_Jet, 0.0, CMType_Linear,{ 0, 0, 127 } },
				{ CMIndex_Jet, 0.1, CMType_Linear,{ 0, 0, 255 } },
				{ CMIndex_Jet, 0.35, CMType_Linear,{ 0, 255, 255 } },
				{ CMIndex_Jet, 0.5, CMType_Linear,{ 0, 255, 0 } },
				{ CMIndex_Jet, 0.65, CMType_Linear,{ 255, 255, 0 } },
				{ CMIndex_Jet, 0.9, CMType_Linear,{ 255, 0, 0 } },
				{ CMIndex_Jet, 1.0, CMType_Linear,{ 127, 0, 0 } },
				// Rainbow
				{ CMIndex_Rainbow, 0.0, CMType_Linear,{ 0, 0, 255 } },
				{ CMIndex_Rainbow, 0.25, CMType_Linear,{ 0, 255, 255 } },
				{ CMIndex_Rainbow, 0.5, CMType_Linear,{ 0, 255, 0 } },
				{ CMIndex_Rainbow, 0.75, CMType_Linear,{ 255, 255, 0 } },
				{ CMIndex_Rainbow, 1.0, CMType_Linear,{ 255, 0, 0 } },
				// Rainbow Wide
				{ CMIndex_RainbowWide, 0.0, CMType_Linear,{ 0, 0, 0 } },
				{ CMIndex_RainbowWide, 0.1, CMType_Linear,{ 0, 0, 255 } },
				{ CMIndex_RainbowWide, 0.3, CMType_Linear,{ 0, 255, 255 } },
				{ CMIndex_RainbowWide, 0.5, CMType_Linear,{ 0, 255, 0 } },
				{ CMIndex_RainbowWide, 0.7, CMType_Linear,{ 255, 255, 0 } },
				{ CMIndex_RainbowWide, 0.9, CMType_Linear,{ 255, 0, 0 } },
				{ CMIndex_RainbowWide, 1.0, CMType_Linear,{ 255, 255, 255 } },
				// Spectrum
				{ CMIndex_Spectrum, 0.0, CMType_Linear,{ 255, 0, 255 } },
				{ CMIndex_Spectrum, 0.1, CMType_Linear,{ 0, 0, 255 } },
				{ CMIndex_Spectrum, 0.3, CMType_Linear,{ 0, 255, 255 } },
				{ CMIndex_Spectrum, 0.45, CMType_Linear,{ 0, 255, 0 } },
				{ CMIndex_Spectrum, 0.6, CMType_Linear,{ 255, 255, 0 } },
				{ CMIndex_Spectrum, 1.0, CMType_Linear,{ 255, 0, 0 } },
				// Spectrum Wide
				{ CMIndex_SpectrumWide, 0.0, CMType_Linear,{ 0, 0, 0 } },
				{ CMIndex_SpectrumWide, 0.1, CMType_Linear,{ 150, 0, 150 } },
				{ CMIndex_SpectrumWide, 0.2, CMType_Linear,{ 0, 0, 255 } },
				{ CMIndex_SpectrumWide, 0.35, CMType_Linear,{ 0, 255, 255 } },
				{ CMIndex_SpectrumWide, 0.5, CMType_Linear,{ 0, 255, 0 } },
				{ CMIndex_SpectrumWide, 0.6, CMType_Linear,{ 255, 255, 0 } },
				{ CMIndex_SpectrumWide, 0.9, CMType_Linear,{ 255, 0, 0 } },
				{ CMIndex_SpectrumWide, 1.0, CMType_Linear,{ 255, 255, 255 } },
				// Thermal
				{ CMIndex_Thermal, 0.0, CMType_Linear,{  0, 0, 255 } },
				{ CMIndex_Thermal, 0.5, CMType_Linear,{ 255, 0, 255 } },
				{ CMIndex_Thermal, 1.0, CMType_Linear,{ 255, 255, 0 } },
				// Thermal Wide
				{ CMIndex_ThermalWide, 0.0, CMType_Linear,{  0, 0, 0 } },
				{ CMIndex_ThermalWide, 0.05, CMType_Linear,{  0, 0, 255 } },
				{ CMIndex_ThermalWide, 0.5, CMType_Linear,{ 255, 0, 255 } },
				{ CMIndex_ThermalWide, 0.95, CMType_Linear,{ 255, 255, 0 } },
				{ CMIndex_ThermalWide, 1.0, CMType_Linear,{ 255, 255, 255 } },

				// Cool Warm
				{ CMIndex_CoolWarm, 0.0, CMType_Diverging,{ 59, 76, 192 } },
				{ CMIndex_CoolWarm, 1.0, CMType_Diverging,{ 180, 4, 38 } },
				// PurpleOrange
				{ CMIndex_PurpleOrange, 0.0, CMType_Diverging,{ 111, 78, 161 } },
				{ CMIndex_PurpleOrange, 1.0, CMType_Diverging,{ 193, 85, 11 } },
				// GreenPurple
				{ CMIndex_GreenPurple, 0.0, CMType_Diverging,{ 21, 135, 51 } },
				{ CMIndex_GreenPurple, 1.0, CMType_Diverging,{ 111, 78, 161 } },
				// Blue DarkYellow
				{ CMIndex_BlueDarkYellow, 0.0, CMType_Diverging,{ 55, 133, 232 } },
				{ CMIndex_BlueDarkYellow, 1.0, CMType_Diverging,{ 172, 125, 23 } },
				// Green Red
				{ CMIndex_GreenRed, 0.0, CMType_Diverging,{ 21, 135, 51 } },
				{ CMIndex_GreenRed, 1.0, CMType_Diverging,{ 193, 54, 59 } },

				// End mark (Don't remove this)
				{CMIndex_End, 0.0, CMType_Linear,{ 0, 0, 0 }}
			};
			const ColorMapData	*colorMapData = sColorMapData;
			
			while (colorMapData->index != inIndex)
			{
				colorMapData++;
				if (colorMapData->index == CMIndex_End)
					return NULL;
			}
			
			return colorMapData;
		}

	};
 };
};

#endif	// #ifdef VIW_UTIL_COLORMAP_H

