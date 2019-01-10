// LyricLine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdlib.h>
#include <fstream>

#define COLOR_DELTA_VOTE 20
#define COLOR_DELTA_SEARCH_LC 5
#define COLOR_DELTA_SEARCH_RC 5
#define COLOR_DELTA_SEARCH_BG 10

struct LineSeg
{
	int start;
	int end;

	LineSeg()
		: start(0), end(0)
	{

	}
};

bool colorSimilar_RGBA(const unsigned char* c1, const unsigned char* c2, int delt)
{
	int d1 = (int)(c2[0] - c1[0]) * (int)(c2[0] - c1[0]);
	int d2 = (int)(c2[1] - c1[1]) * (int)(c2[1] - c1[1]);
	int d3 = (int)(c2[2] - c1[2]) * (int)(c2[2] - c1[2]);
	int total = d1 + d2 + d3;
	return total < delt * delt;
}

float pixelVotedRatio(const unsigned char* beVotedColor, const unsigned char* begin, const unsigned char* stopedColor, int count_max, int step)
{
	int count = 0;
	int voted = 0;
	while(count < count_max)
	{
		// 如果当前颜色与停止颜色很接近，则处理停止
		if(colorSimilar_RGBA(begin, stopedColor, COLOR_DELTA_SEARCH_BG))
			break;
		if(colorSimilar_RGBA(begin, beVotedColor, COLOR_DELTA_VOTE))
			voted++;
		begin += (4*step);
		count++;
	}
	return count<=0 ? 0.0f : (float)voted / count;
}

int getLyricLineSeg_line(const unsigned char* pLineBuffer, int w, int pitch, unsigned int bgcolor, LineSeg& seg)
{
// 	unsigned int c1 = 0xFFFFFFFF;
// 	unsigned int c2 = 0;
// 	colorSimilar_RGBA((const unsigned char*)&c1, (const unsigned char*)&c2, 10);
	// 扫描一行中的seg.start -- seg.end区间，尝试找到分割线
	const unsigned char* line_start = pLineBuffer + seg.start * 4;
	const unsigned char* line_end = pLineBuffer + (seg.end-1)*4;
	unsigned int lc = 0;
	unsigned int rc = 0;
	int seg_left = seg.start;
	int seg_right = seg.end; 
	int result_left = seg.start;
	int result_right = seg.end;
	//for(int i=0; i<(seg.end - seg.start) / 2; i++)
	for(int i=seg.start; i<seg.end; i++)
	{
		unsigned int leftColor = *(unsigned int*)line_start;
		//if(!colorSimilar_RGBA(line_start, (unsigned char*)&bgcolor, 10)/*leftColor!=bgcolor*/)		//应该是在某个区间内都认为相等
		{
			if (lc==0)
			{
				if(!colorSimilar_RGBA(line_start, (unsigned char*)&bgcolor, COLOR_DELTA_SEARCH_BG))
				{
					// fixme 此时应当往前嗅探，看看能找到与当前颜色相同的颜色，周围存在越多与此像素颜色相近的值则此颜色选中的概率应该更高
					// 类似由周围的像素来投票当前像素是否可以被选中
					float votedRatio = pixelVotedRatio(line_start, line_start+4, (unsigned char*)&bgcolor, 20, 1);
					if(votedRatio>=0.6f)
					{
						lc = leftColor;
						result_left = seg_left;
					}
				}
			}
			else if(colorSimilar_RGBA(line_start, (unsigned char*)&lc, COLOR_DELTA_SEARCH_LC))
			{
				result_left = seg_left;
			}
			seg_left++;
		}
		line_start += 4;
	}
	for(int i=seg.start; i<seg.end; i++)
	{
		unsigned int rightColor = *(unsigned int*)line_end;
		//if(!colorSimilar_RGBA(line_end, (unsigned char*)&bgcolor, 10)/*rightColor!=bgcolor*/)
		{
			if (rc==0)
			{
				if(!colorSimilar_RGBA(line_end, (unsigned char*)&bgcolor, COLOR_DELTA_SEARCH_BG))
				{
					float votedRatio = pixelVotedRatio(line_end, line_end-4, (unsigned char*)&bgcolor, 20, -1);
					if(votedRatio>=0.6f)
					{
						rc = rightColor;
						result_right = seg_right;
					}
				}
			}
			else if(colorSimilar_RGBA(line_end, (unsigned char*)&rc, COLOR_DELTA_SEARCH_RC))
			{
				result_right = seg_right;
			}
			seg_right--;
		}
		line_end -= 4;
	}
	seg.start = result_left;
	seg.end = result_right;
	return 0;
}

int getLyricLineSeg(const unsigned char* pictureBuffer, int w, int h, int pitch, unsigned int bgcolor, LineSeg& seg)
{
	int middle_line = (h + 1) / 2;
	const unsigned char* curline = pictureBuffer + pitch * middle_line;
	for (int i=0; i<h; i++)
	{
		const unsigned char* curline = pictureBuffer + pitch * i;
		int ret = getLyricLineSeg_line(curline, w, pitch, bgcolor, seg);
	}
// 	const unsigned char* line_start = curline + seg.start * 4;
// 	const unsigned char* line_end = curline + (seg.end-1)*4;
// 	unsigned int lc = 0;
// 	unsigned int rc = 0;
// 	int seg_left = seg.start;
// 	int seg_right = seg.end; 
// 	for(int i=0; i<(seg.end - seg.start) / 2; i++)
// 	{
// 		unsigned int leftColor = *(unsigned int*)line_start;
// 		unsigned int rightColor = *(unsigned int*)line_end;
// 		if(leftColor!=bgcolor)
// 		{
// 			if (lc==0)
// 			{
// 				lc = leftColor;
// 				seg_left++;
// 			}
// 			else if(leftColor!=rc)
// 			{
// 				seg_left++;
// 			}
// 		}
// 		if(rightColor!=bgcolor)
// 		{
// 			if (rc==0)
// 			{
// 				rc = rightColor;
// 				seg_right--;
// 			}
// 			else if(rightColor!=lc)
// 			{
// 				seg_right--;
// 			}
// 		}
// 		line_start += 4;
// 		line_end -= 4;
// 	}
// 	seg.start = seg_left;
// 	seg.end = seg_right;
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int w = 980;
	int h = 141;
	int pitch = w * 4;
	unsigned char* picBuffer = (unsigned char*)malloc(pitch * h);
	std::ifstream lyricImg(argv[1], std::ios::binary);
	lyricImg.read((char*)picBuffer, pitch * h);
	LineSeg regResult;
	regResult.start = 0;
	regResult.end = w;
	unsigned int bgcolor = *(unsigned int*)picBuffer;
	int ret = getLyricLineSeg(picBuffer, w, h, pitch, bgcolor, regResult);
	return 0;
}

