#include "ui.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize.h"

#include <imgui/imgui_internal.h>

#include "fw-imgui/imgui_impl_dx11.h"
#include "fw-imgui/imgui_impl_dx12.h"
#include "fw-imgui/imgui_impl_win32.h"

#include "ModFramework.hpp"

#include <sstream>
#include <comdef.h>
#include <iomanip>

// clang-format off

// Those MIN/MAX values are not define because we need to point to them
static const signed char    IM_S8_MIN = -128;
static const signed char    IM_S8_MAX = 127;
static const unsigned char  IM_U8_MIN = 0;
static const unsigned char  IM_U8_MAX = 0xFF;
static const signed short   IM_S16_MIN = -32768;
static const signed short   IM_S16_MAX = 32767;
static const unsigned short IM_U16_MIN = 0;
static const unsigned short IM_U16_MAX = 0xFFFF;
static const ImS32          IM_S32_MIN = INT_MIN;    // (-2147483647 - 1), (0x80000000);
static const ImS32          IM_S32_MAX = INT_MAX;    // (2147483647), (0x7FFFFFFF)
static const ImU32          IM_U32_MIN = 0;
static const ImU32          IM_U32_MAX = UINT_MAX;   // (0xFFFFFFFF)
#ifdef LLONG_MIN
static const ImS64          IM_S64_MIN = LLONG_MIN;  // (-9223372036854775807ll - 1ll);
static const ImS64          IM_S64_MAX = LLONG_MAX;  // (9223372036854775807ll);
#else
static const ImS64          IM_S64_MIN = -9223372036854775807LL - 1;
static const ImS64          IM_S64_MAX = 9223372036854775807LL;
#endif
static const ImU64          IM_U64_MIN = 0;
#ifdef ULLONG_MAX
static const ImU64          IM_U64_MAX = ULLONG_MAX; // (0xFFFFFFFFFFFFFFFFull);
#else
static const ImU64          IM_U64_MAX = (2ULL * 9223372036854775807LL + 1);
#endif

unsigned int UI::stb_decompress_length(const unsigned char *input)
{
    return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *stb__barrier_out_e, *stb__barrier_out_b;
static const unsigned char *stb__barrier_in_b;
static unsigned char *stb__dout;
static void stb__match(const unsigned char *data, unsigned int length)
{
    // INVERSE of memmove... write each byte before copying the next...
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_out_b) { stb__dout = stb__barrier_out_e+1; return; }
    while (length--) *stb__dout++ = *data++;
}

static void stb__lit(const unsigned char *data, unsigned int length)
{
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_in_b) { stb__dout = stb__barrier_out_e+1; return; }
    memcpy(stb__dout, data, length);
    stb__dout += length;
}

#define stb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define stb__in3(x)   ((i[x] << 16) + stb__in2((x)+1))
#define stb__in4(x)   ((i[x] << 24) + stb__in3((x)+1))

static const unsigned char *stb_decompress_token(const unsigned char *i)
{
    if (*i >= 0x20) { // use fewer if's for cases that expand small
        if (*i >= 0x80)       stb__match(stb__dout-i[1]-1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)  stb__match(stb__dout-(stb__in2(0) - 0x4000 + 1), i[2]+1), i += 3;
        else /* *i >= 0x20 */ stb__lit(i+1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else { // more ifs for cases that expand large, since overhead is amortized
        if (*i >= 0x18)       stb__match(stb__dout-(stb__in3(0) - 0x180000 + 1), i[3]+1), i += 4;
        else if (*i >= 0x10)  stb__match(stb__dout-(stb__in3(0) - 0x100000 + 1), stb__in2(3)+1), i += 5;
        else if (*i >= 0x08)  stb__lit(i+2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)  stb__lit(i+3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
        else if (*i == 0x06)  stb__match(stb__dout-(stb__in3(1)+1), i[4]+1), i += 5;
        else if (*i == 0x04)  stb__match(stb__dout-(stb__in3(1)+1), stb__in2(4)+1), i += 6;
    }
    return i;
}

static unsigned int stb_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen = buflen % 5552;

    unsigned long i;
    while (buflen) {
        for (i=0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;

            buffer += 8;
        }

        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;

        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

static ImVec2 operator*(const ImVec2& lhs, const float rhs)              { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
static ImVec2 operator/(const ImVec2& lhs, const float rhs)              { return ImVec2(lhs.x / rhs, lhs.y / rhs); }
static ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
static ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
static ImVec2 operator/(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
static ImVec2& operator*=(ImVec2& lhs, const float rhs)                  { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
static ImVec2& operator/=(ImVec2& lhs, const float rhs)                  { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
static ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
static ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
static ImVec2& operator*=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
static ImVec2& operator/=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }
static ImVec4 operator+(const ImVec4& lhs, const ImVec4& rhs)            { return ImVec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
static ImVec4 operator-(const ImVec4& lhs, const ImVec4& rhs)            { return ImVec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
static ImVec4 operator*(const ImVec4& lhs, const ImVec4& rhs)            { return ImVec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }

static const char* PatchFormatStringFloatToInt(const char* fmt) {
  if (fmt[0] == '%' && fmt[1] == '.' && fmt[2] == '0' && fmt[3] == 'f' &&
      fmt[4] == 0) // Fast legacy path for "%.0f" which is expected to be the
                   // most common case.
    return "%d";
  const char* fmt_start =
      ImParseFormatFindStart(fmt); // Find % (if any, and ignore %%)
  const char* fmt_end = ImParseFormatFindEnd(
      fmt_start); // Find end of format specifier, which itself is an exercise
                  // of confidence/recklessness (because snprintf is dependent
                  // on libc or user).
  if (fmt_end > fmt_start && fmt_end[-1] == 'f') {
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    if (fmt_start == fmt && fmt_end[0] == 0)
      return "%d";
    ImGuiContext& g = *GImGui;
    ImFormatString(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), "%.*s%%d%s",
                   (int)(fmt_start - fmt), fmt,
                   fmt_end); // Honor leading and trailing decorations, but lose
                             // alignment/precision.
    return g.TempBuffer;
#else
    IM_ASSERT(
        0 &&
        "DragInt(): Invalid format string!"); // Old versions used a default
                                              // parameter of "%.0f", please
                                              // replace with e.g. "%d"
#endif
  }
  return fmt;
}

unsigned int UI::stb_decompress(unsigned char *output, const unsigned char *i, unsigned int length)
{
    if (stb__in4(0) != 0x57bC0000) return 0;
    if (stb__in4(4) != 0)          return 0; // error! stream is > 4GB
    const unsigned int olen = stb_decompress_length(i);
    stb__barrier_in_b = i;
    stb__barrier_out_e = output + olen;
    stb__barrier_out_b = output;
    i += 16;

    stb__dout = output;
    for (;;) {
        const unsigned char *old_i = i;
        i = stb_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                IM_ASSERT(stb__dout == output + olen);
                if (stb__dout != output + olen) return 0;
                if (stb_adler32(1, output, olen) != (unsigned int) stb__in4(2))
                    return 0;
                return olen;
            } else {
                IM_ASSERT(0); /* NOTREACHED */
                return 0;
            }
        }
        IM_ASSERT(stb__dout <= output + olen);
        if (stb__dout > output + olen)
            return 0;
    }
}

int UI::stb_resize(const unsigned char* input_pixels, int input_w, int input_h, int input_stride_in_bytes,
    unsigned char* output_pixels, int output_w, int output_h, int output_stride_in_bytes,
    int num_channels) 
{
    return stbir_resize_uint8(input_pixels, input_w, input_h, input_stride_in_bytes, 
        output_pixels, output_w, output_h, output_stride_in_bytes,
        num_channels);
}

bool UI_TempInputScalar(const ImRect& bb, ImGuiID id, const char* label, ImGuiDataType data_type, void* p_data, const char* format, const void* p_clamp_min, const void* p_clamp_max)
{
    using namespace ImGui;

	ImGuiContext& g = *GImGui;

	char fmt_buf[32];
	char data_buf[32];
	format = ImParseFormatTrimDecorations(format, fmt_buf, IM_ARRAYSIZE(fmt_buf));
	DataTypeFormatString(data_buf, IM_ARRAYSIZE(data_buf), data_type, p_data, format);
	ImStrTrimBlanks(data_buf);

	ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_NoMarkEdited;
	flags |= ((data_type == ImGuiDataType_Float || data_type == ImGuiDataType_Double) ? ImGuiInputTextFlags_CharsScientific : ImGuiInputTextFlags_CharsDecimal);
	bool value_changed = false;

    float text_input_height = bb.Max.y - bb.Min.y;
    float min_input_height = CalcTextSize("0123456789").y + (CalcTextSize("0123456789").y / 3);
    text_input_height < min_input_height ? text_input_height = min_input_height : text_input_height = text_input_height;
    float text_input_min_height = (bb.Min.y + bb.Max.y) / 2 - text_input_height / 2;
    ImRect text_input_bb(ImVec2(bb.Min.x, text_input_min_height), ImVec2(bb.Max.x, text_input_min_height + text_input_height));

    if (TempInputText(text_input_bb, id, label, data_buf, IM_ARRAYSIZE(data_buf), flags))
	{
		// Backup old value
		size_t data_type_size = DataTypeGetInfo(data_type)->Size;
		ImGuiDataTypeTempStorage data_backup;
		memcpy(&data_backup, p_data, data_type_size);

		// Apply new value (or operations) then clamp
		DataTypeApplyOpFromText(data_buf, g.InputTextState.InitialTextA.Data, data_type, p_data, nullptr);
		if (p_clamp_min || p_clamp_max)
		{
			if (p_clamp_min && p_clamp_max && DataTypeCompare(data_type, p_clamp_min, p_clamp_max) > 0)
				ImSwap(p_clamp_min, p_clamp_max);
			DataTypeClamp(data_type, p_data, p_clamp_min, p_clamp_max);
		}
		// Only mark as edited if new value is diffe
		value_changed = memcmp(&data_backup, p_data, data_type_size) != 0;
		if (value_changed)
			MarkItemEdited(id);
	}
	return value_changed;
}

// FIXME: Move more of the code into SliderBehavior()
template<typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
bool UI_SliderBehaviorT(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, TYPE* v, const TYPE v_min, const TYPE v_max, const char* format, ImGuiSliderFlags flags, ImRect* out_grab_bb, float grab_thickness)
{
    using namespace ImGui;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	const ImGuiAxis axis = (flags & ImGuiSliderFlags_Vertical) ? ImGuiAxis_Y : ImGuiAxis_X;
	const bool is_logarithmic = (flags & ImGuiSliderFlags_Logarithmic) != 0;
	const bool is_floating_point = (data_type == ImGuiDataType_Float) || (data_type == ImGuiDataType_Double);

	const float grab_padding = 2.0f;
	const float slider_sz = (bb.Max[axis] - bb.Min[axis]) - grab_padding * 2.0f;
	float grab_sz = style.GrabMinSize;
	SIGNEDTYPE v_range = (v_min < v_max ? v_max - v_min : v_min - v_max);
    if (!is_floating_point && v_range >= 0)                                             // v_range < 0 may happen on integer overflows
        grab_sz = ImMax((float)(grab_thickness/*slider_sz / (v_range + 1)*/), style.GrabMinSize);  // For integer sliders: if possible have the grab size represent 1 unit
	else
        grab_sz = grab_thickness;
    grab_sz = ImMin(grab_sz, slider_sz);
	const float slider_usable_sz = slider_sz - grab_sz;
	const float slider_usable_pos_min = bb.Min[axis] + grab_padding + grab_sz * 0.5f;
	const float slider_usable_pos_max = bb.Max[axis] - grab_padding - grab_sz * 0.5f;

	float logarithmic_zero_epsilon = 0.0f; // Only valid when is_logarithmic is true
	float zero_deadzone_halfsize = 0.0f; // Only valid when is_logarithmic is true
	if (is_logarithmic)
	{
		// When using logarithmic sliders, we need to clamp to avoid hitting zero, but our choice of clamp value greatly affects slider precision. We attempt to use the specified precision to estimate a good lower bound.
		const int decimal_precision = is_floating_point ? ImParseFormatPrecision(format, 3) : 1;
		logarithmic_zero_epsilon = ImPow(0.1f, (float)decimal_precision);
		zero_deadzone_halfsize = (style.LogSliderDeadzone * 0.5f) / ImMax(slider_usable_sz, 1.0f);
	}

	// Process interacting with the slider
	bool value_changed = false;
	if (g.ActiveId == id)
	{
		bool set_new_value = false;
		float clicked_t = 0.0f;
		if (g.ActiveIdSource == ImGuiInputSource_Mouse)
		{
			if (!g.IO.MouseDown[0])
			{
				ClearActiveID();
			}
			else
			{
				const float mouse_abs_pos = g.IO.MousePos[axis];
				clicked_t = (slider_usable_sz > 0.0f) ? ImClamp((mouse_abs_pos - slider_usable_pos_min) / slider_usable_sz, 0.0f, 1.0f) : 0.0f;
				if (axis == ImGuiAxis_Y)
					clicked_t = 1.0f - clicked_t;
				set_new_value = true;
			}
		}
		else if (g.ActiveIdSource == ImGuiInputSource_Nav)
		{
			if (g.ActiveIdIsJustActivated)
			{
				g.SliderCurrentAccum = 0.0f; // Reset any stored nav delta upon activation
				g.SliderCurrentAccumDirty = false;
			}

			const ImVec2 input_delta2 = GetNavInputAmount2d(ImGuiNavDirSourceFlags_Keyboard | ImGuiNavDirSourceFlags_PadDPad, ImGuiInputReadMode_RepeatFast, 0.0f, 0.0f);
			float input_delta = (axis == ImGuiAxis_X) ? input_delta2.x : -input_delta2.y;
			if (input_delta != 0.0f)
			{
				const int decimal_precision = is_floating_point ? ImParseFormatPrecision(format, 3) : 0;
				if (decimal_precision > 0)
				{
					input_delta /= 100.0f;    // Gamepad/keyboard tweak speeds in % of slider bounds
					if (IsNavInputDown(ImGuiNavInput_TweakSlow))
						input_delta /= 10.0f;
				}
				else
				{
					if ((v_range >= -100.0f && v_range <= 100.0f) || IsNavInputDown(ImGuiNavInput_TweakSlow))
						input_delta = ((input_delta < 0.0f) ? -1.0f : +1.0f) / (float)v_range; // Gamepad/keyboard tweak speeds in integer steps
					else
						input_delta /= 100.0f;
				}
				if (IsNavInputDown(ImGuiNavInput_TweakFast))
					input_delta *= 10.0f;

				g.SliderCurrentAccum += input_delta;
				g.SliderCurrentAccumDirty = true;
			}

			float delta = g.SliderCurrentAccum;
			if (g.NavActivatePressedId == id && !g.ActiveIdIsJustActivated)
			{
				ClearActiveID();
			}
			else if (g.SliderCurrentAccumDirty)
			{
				clicked_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, *v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);

				if ((clicked_t >= 1.0f && delta > 0.0f) || (clicked_t <= 0.0f && delta < 0.0f)) // This is to avoid applying the saturation when already past the limits
				{
					set_new_value = false;
					g.SliderCurrentAccum = 0.0f; // If pushing up against the limits, don't continue to accumulate
				}
				else
				{
					set_new_value = true;
					float old_clicked_t = clicked_t;
					clicked_t = ImSaturate(clicked_t + delta);

					// Calculate what our "new" clicked_t will be, and thus how far we actually moved the slider, and subtract this from the accumulator
					TYPE v_new = ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, clicked_t, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
					if (!(flags & ImGuiSliderFlags_NoRoundToFormat))
						v_new = RoundScalarWithFormatT<TYPE, SIGNEDTYPE>(format, data_type, v_new);
					float new_clicked_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, v_new, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);

					if (delta > 0)
						g.SliderCurrentAccum -= ImMin(new_clicked_t - old_clicked_t, delta);
					else
						g.SliderCurrentAccum -= ImMax(new_clicked_t - old_clicked_t, delta);
				}

				g.SliderCurrentAccumDirty = false;
			}
		}

		if (set_new_value)
		{
			TYPE v_new = ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, clicked_t, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);

			// Round to user desired precision based on format string
			if (!(flags & ImGuiSliderFlags_NoRoundToFormat))
				v_new = RoundScalarWithFormatT<TYPE, SIGNEDTYPE>(format, data_type, v_new);

			// Apply result
			if (*v != v_new)
			{
				*v = v_new;
				value_changed = true;
			}
		}
	}

	if (slider_sz < 1.0f)
	{
		*out_grab_bb = ImRect(bb.Min, bb.Min);
	}
	else
	{
		// Output grab position so it can be displayed by the caller
		float grab_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, *v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
		if (axis == ImGuiAxis_Y)
			grab_t = 1.0f - grab_t;
		const float grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
		if (axis == ImGuiAxis_X)
			*out_grab_bb = ImRect(grab_pos - grab_sz * 0.5f, bb.Min.y + grab_padding - 5.0f, grab_pos + grab_sz * 0.5f, bb.Max.y - grab_padding + 5.0);
		else
			*out_grab_bb = ImRect(bb.Min.x + grab_padding, grab_pos - grab_sz * 0.5f - 5.0f, bb.Max.x - grab_padding, grab_pos + grab_sz * 0.5f + 5.0f);
	}

	return value_changed;
}

bool UI_SliderBehavior(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, void* p_v, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags, ImRect* out_grab_bb, float grab_thickness)
{
	// Read imgui.cpp "API BREAKING CHANGES" section for 1.78 if you hit this assert.
	IM_ASSERT((flags == 1 || (flags & ImGuiSliderFlags_InvalidMask_) == 0) && "Invalid ImGuiSliderFlags flag!  Has the 'float power' argument been mistakenly cast to flags? Call function with ImGuiSliderFlags_Logarithmic flags instead.");

	ImGuiContext& g = *GImGui;
	if ((g.CurrentItemFlags & ImGuiItemFlags_ReadOnly) || (flags & ImGuiSliderFlags_ReadOnly))
		return false;

	switch (data_type)
	{
	case ImGuiDataType_S8: { ImS32 v32 = (ImS32) * (ImS8*)p_v;  bool r = UI_SliderBehaviorT<ImS32, ImS32, float>(bb, id, ImGuiDataType_S32, &v32, *(const ImS8*)p_min, *(const ImS8*)p_max, format, flags, out_grab_bb, grab_thickness); if (r) *(ImS8*)p_v = (ImS8)v32;  return r; }
	case ImGuiDataType_U8: { ImU32 v32 = (ImU32) * (ImU8*)p_v;  bool r = UI_SliderBehaviorT<ImU32, ImS32, float>(bb, id, ImGuiDataType_U32, &v32, *(const ImU8*)p_min, *(const ImU8*)p_max, format, flags, out_grab_bb, grab_thickness); if (r) *(ImU8*)p_v = (ImU8)v32;  return r; }
	case ImGuiDataType_S16: { ImS32 v32 = (ImS32) * (ImS16*)p_v; bool r = UI_SliderBehaviorT<ImS32, ImS32, float>(bb, id, ImGuiDataType_S32, &v32, *(const ImS16*)p_min, *(const ImS16*)p_max, format, flags, out_grab_bb, grab_thickness); if (r) *(ImS16*)p_v = (ImS16)v32; return r; }
	case ImGuiDataType_U16: { ImU32 v32 = (ImU32) * (ImU16*)p_v; bool r = UI_SliderBehaviorT<ImU32, ImS32, float>(bb, id, ImGuiDataType_U32, &v32, *(const ImU16*)p_min, *(const ImU16*)p_max, format, flags, out_grab_bb, grab_thickness); if (r) *(ImU16*)p_v = (ImU16)v32; return r; }
	case ImGuiDataType_S32:
		IM_ASSERT(*(const ImS32*)p_min >= IM_S32_MIN / 2 && *(const ImS32*)p_max <= IM_S32_MAX / 2);
		return UI_SliderBehaviorT<ImS32, ImS32, float >(bb, id, data_type, (ImS32*)p_v, *(const ImS32*)p_min, *(const ImS32*)p_max, format, flags, out_grab_bb, grab_thickness);
	case ImGuiDataType_U32:
		IM_ASSERT(*(const ImU32*)p_max <= IM_U32_MAX / 2);
		return UI_SliderBehaviorT<ImU32, ImS32, float >(bb, id, data_type, (ImU32*)p_v, *(const ImU32*)p_min, *(const ImU32*)p_max, format, flags, out_grab_bb, grab_thickness);
	case ImGuiDataType_S64:
		IM_ASSERT(*(const ImS64*)p_min >= IM_S64_MIN / 2 && *(const ImS64*)p_max <= IM_S64_MAX / 2);
		return UI_SliderBehaviorT<ImS64, ImS64, double>(bb, id, data_type, (ImS64*)p_v, *(const ImS64*)p_min, *(const ImS64*)p_max, format, flags, out_grab_bb, grab_thickness);
	case ImGuiDataType_U64:
		IM_ASSERT(*(const ImU64*)p_max <= IM_U64_MAX / 2);
		return UI_SliderBehaviorT<ImU64, ImS64, double>(bb, id, data_type, (ImU64*)p_v, *(const ImU64*)p_min, *(const ImU64*)p_max, format, flags, out_grab_bb, grab_thickness);
	case ImGuiDataType_Float:
		IM_ASSERT(*(const float*)p_min >= -FLT_MAX / 2.0f && *(const float*)p_max <= FLT_MAX / 2.0f);
		return UI_SliderBehaviorT<float, float, float >(bb, id, data_type, (float*)p_v, *(const float*)p_min, *(const float*)p_max, format, flags, out_grab_bb, grab_thickness);
	case ImGuiDataType_Double:
		IM_ASSERT(*(const double*)p_min >= -DBL_MAX / 2.0f && *(const double*)p_max <= DBL_MAX / 2.0f);
		return UI_SliderBehaviorT<double, double, double>(bb, id, data_type, (double*)p_v, *(const double*)p_min, *(const double*)p_max, format, flags, out_grab_bb, grab_thickness);
	case ImGuiDataType_COUNT: break;
	}
	IM_ASSERT(0);
	return false;
}

bool UI_SliderScalar(const char* label, ImGuiDataType data_type, void* p_data,
                  const void* p_min, const void* p_max, const char* format,
                  float thickness, float grab_thickness, ImGuiSliderFlags flags) {

  using namespace ImGui;
  ImGuiWindow* window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ImGuiContext& g         = *GImGui;
  const ImGuiStyle& style = g.Style;
  const ImGuiID id        = window->GetID(label);
  const float w           = CalcItemWidth();

  const ImVec2 label_size = thickness == NULL ? CalcTextSize(label, NULL, true) : ImVec2(0.0f , thickness);
  const ImRect frame_bb(
      window->DC.CursorPos,
      window->DC.CursorPos +
          ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
  const ImRect total_bb(frame_bb.Min,
                        frame_bb.Max +
                            ImVec2(label_size.x > 0.0f
                                       ? style.ItemInnerSpacing.x + label_size.x
                                       : 0.0f,
                                   0.0f));

  const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
  ItemSize(total_bb, style.FramePadding.y);
  if (!ItemAdd(total_bb, id, &frame_bb,
                      temp_input_allowed ? ImGuiItemAddFlags_Focusable : 0))
    return false;

  // Default format string when passing NULL
  if (format == NULL)
    format = DataTypeGetInfo(data_type)->PrintFmt;
  else if (data_type == ImGuiDataType_S32 &&
           strcmp(format, "%d") !=
               0) // (FIXME-LEGACY: Patch old "%.0f" format string to use "%d",
                  // read function more details.)
    format = PatchFormatStringFloatToInt(format);

  // Tabbing or CTRL-clicking on Slider turns it into an input box
  const bool hovered = ItemHoverable(frame_bb, id);
  bool temp_input_is_active =
      temp_input_allowed && TempInputIsActive(id);
  if (!temp_input_is_active) {
    const bool focus_requested =
        temp_input_allowed &&
        (window->DC.LastItemStatusFlags & ImGuiItemStatusFlags_Focused) != 0;
    const bool clicked = (hovered && g.IO.MouseClicked[0]);
    if (focus_requested || clicked || g.NavActivateId == id ||
        g.NavInputId == id) {
      SetActiveID(id, window);
      SetFocusID(id, window);
      FocusWindow(window);
      g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
      if (temp_input_allowed &&
          (focus_requested || (clicked && g.IO.KeyCtrl) || g.NavInputId == id))
        temp_input_is_active = true;
    }
  }

  if (temp_input_is_active) {
    // Only clamp CTRL+Click input when ImGuiSliderFlags_AlwaysClamp is set
    const bool is_clamp_input = (flags & ImGuiSliderFlags_AlwaysClamp) != 0;
    return UI_TempInputScalar(frame_bb, id, label, data_type, p_data,
                                  format, is_clamp_input ? p_min : NULL,
                                  is_clamp_input ? p_max : NULL);
  }

  // Draw frame
  const ImU32 frame_col =
      GetColorU32(g.ActiveId == id    ? ImGuiCol_FrameBgActive
                         : g.HoveredId == id ? ImGuiCol_FrameBgHovered
                                             : ImGuiCol_FrameBg);
  RenderNavHighlight(frame_bb, id);
  RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true,
                     g.Style.FrameRounding);

  // Slider behavior
  ImRect grab_bb;
  const bool value_changed = UI_SliderBehavior(frame_bb, id, data_type, p_data, p_min, p_max, format, flags, &grab_bb, grab_thickness);
  if (value_changed)
	  MarkItemEdited(id);

  // Render fill area and add sex appeal to the grab
  if (memcmp(p_data, p_min, DataTypeGetInfo(data_type)->Size) == 0) {
	  if (grab_bb.Max.x > grab_bb.Min.x)
		  window->DrawList->AddRectFilled(ImVec2(total_bb.Min.x + 1.0f, grab_bb.Min.y + 5.0f), ImVec2(grab_bb.Min.x, grab_bb.Max.y - 5.0f), ImGui::GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);
  }
  else {
	  if (grab_bb.Max.x > grab_bb.Min.x)
		  window->DrawList->AddRectFilled(ImVec2(total_bb.Min.x + 1.0f, grab_bb.Min.y + 5.0f), ImVec2(grab_bb.Min.x, grab_bb.Max.y - 5.0f), GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);
  }

  // Render grab
  if (grab_bb.Max.x > grab_bb.Min.x)
	  window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);

  // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
  //char value_buf[64];
  //const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
  if (g.LogEnabled)
	  LogSetNextTextDecoration("{", "}");
  //RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));

  if (label_size.x > 0.0f)
	  RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

  // Slider behavior
  /*ImRect grab_bb;
  const bool value_changed = UI_SliderBehavior(
      frame_bb, id, data_type, p_data, p_min, p_max, format, flags, &grab_bb);
  if (value_changed)
    MarkItemEdited(id);

  // Render fill area and add sex appeal to the grab
  if(memcmp(p_data, p_min, DataTypeGetInfo(data_type)->Size) == 0){
    if (grab_bb.Max.x > grab_bb.Min.x)
      window->DrawList->AddRectFilled(ImVec2(total_bb.Min.x + 1.0f, grab_bb.Min.y), ImVec2(grab_bb.Max.x - 5.0f, grab_bb.Max.y), ImGui::GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive: ImGuiCol_SliderGrab), style.GrabRounding);
    grab_bb.Min += ImVec2(0.0f, -(thickness + 5.0f));
    grab_bb.Max += ImVec2(-5.0f, thickness + 5.0f);
  }
  else {
    if (grab_bb.Max.x > grab_bb.Min.x)
        window->DrawList->AddRectFilled(ImVec2(total_bb.Min.x + 1.0f, grab_bb.Min.y), grab_bb.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive: ImGuiCol_SliderGrab), style.GrabRounding);
    grab_bb.Min += ImVec2(5.0f, -(thickness + 5.0f));
    grab_bb.Max += ImVec2(0.0f, thickness + 5.0f);
  }

  // Render grab
  if (grab_bb.Max.x > grab_bb.Min.x)
    window->DrawList->AddRectFilled(
        grab_bb.Min, grab_bb.Max,
        GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab),
        style.GrabRounding);

  // Display value using user-provided display format so user can add
  // prefix/suffix/decorations to the value.
  char value_buf[64];

  if (g.LogEnabled)
    LogSetNextTextDecoration("{", "}");

  / *if (label_size.x > 0.0f)
    RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x,
                             frame_bb.Min.y + style.FramePadding.y),
                      label);* /*/

  IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
  return value_changed;
}

void UI::RenderRoundTopFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding) 
{
    using namespace ImGui;

	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	window->DrawList->AddRectFilled(p_min, p_max, fill_col, rounding, ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight);
	const float border_size = g.Style.FrameBorderSize;
	if (border && border_size > 0.0f) {
		window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1),
			GetColorU32(ImGuiCol_BorderShadow), rounding, ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight,
			border_size);
		window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border),
			rounding, ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight, border_size);
	}
}

bool UI::TopRoundButton(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
    using namespace ImGui;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(size, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (g.CurrentItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderNavHighlight(bb, id);
	RenderRoundTopFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

	if (g.LogEnabled)
		LogSetNextTextDecoration("[", "]");
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
	return pressed;
}

bool UI::SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, float thickness, ImGuiSliderFlags flags) {
	bool ret;

    float cursorX = ImGui::GetCursorPosX();

    if(label[0] != '#' && label[1] != '#'){
        ImGui::Text(label);
        cursorX += ImGui::CalcTextSize("X").x;
    }

    ImVec2 defCursorPos = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(cursorX, ImGui::GetCursorPosY() + ImGui::CalcTextSize(label).y / 2));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_Border, OUTLINE_GRAY);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.01f, 0.01f, 0.01f, 0.90f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.50f, 0.93f, 0.93f, 0.65f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.24f, 0.74f, 0.88f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.15f, 0.65f, 0.95f, 1.0f));
    ret = UI_SliderScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format, thickness, 5.0f, flags);
    ImGui::SameLine(); ImGui::SetCursorPosY(defCursorPos.y);
    ImGui::SetNextItemWidth(50.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.0f, 0.0f });
    ImGui::Text("Value: ");
	ImGui::SameLine();
	ImGui::Text(format, *v);
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(5);
    return ret;
}

bool UI::SliderInt(const char* label, int* v, int v_min, int v_max, const char* format /*= "%d"*/, float thickness /*= 1.0f*/, ImGuiSliderFlags flags /*= 0*/)
{
	bool ret;

    float cursorX = ImGui::GetCursorPosX();

    if(label[0] != '#' && label[1] != '#'){
        ImGui::Text(label);
        cursorX += ImGui::CalcTextSize("X").x;
    }

    ImVec2 defCursorPos = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(cursorX, ImGui::GetCursorPosY() + ImGui::CalcTextSize(label).y / 2));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_Border, OUTLINE_GRAY);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.01f, 0.01f, 0.01f, 0.90f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.50f, 0.93f, 0.93f, 0.65f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.24f, 0.74f, 0.88f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.15f, 0.65f, 0.95f, 1.0f));
    ret = UI_SliderScalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format, thickness, 5.0f, flags);
    ImGui::SameLine(); ImGui::SetCursorPosY(defCursorPos.y);
    ImGui::SetNextItemWidth(50.0f);
    ImGui::Text("Value: %d", *v);
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(5);
    return ret;
}

bool UI::ModCheckBox(const char* label, bool vDependency, bool* vUser)
{
	using namespace ImGui;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	const float square_sz = GetFrameHeight();
	const ImVec2 pos = window->DC.CursorPos;
	const ImRect total_bb(pos, pos + ImVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));
	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, id))
	{
		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
		return false;
	}

	bool hovered, held;
	bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
	if (pressed)
	{
		*vUser = !(*vUser);
		MarkItemEdited(id);
	}

	const ImRect check_bb(pos, pos + ImVec2(square_sz, square_sz));
	RenderNavHighlight(total_bb, id);
	RenderFrame(check_bb.Min, check_bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);
	ImU32 check_col = GetColorU32(ImGuiCol_CheckMark);
	bool mixed_value = (g.CurrentItemFlags & ImGuiItemFlags_MixedValue) != 0;
	if (mixed_value)
	{
		// Undocumented tristate/mixed/indeterminate checkbox (#2644)
		// This may seem awkwardly designed because the aim is to make ImGuiItemFlags_MixedValue supported by all widgets (not just checkbox)
		ImVec2 pad(ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)), ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)));
		window->DrawList->AddRectFilled(check_bb.Min + pad, check_bb.Max - pad, check_col, style.FrameRounding);
	}
	else if ((*vUser && vDependency) || *vUser)
	{
		const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));
		RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), check_col, square_sz - pad * 2.0f);
	}
	else if (vDependency) {
		const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));
		ImU32 unsat_check_col = GetColorU32(ImGuiCol_CheckMark, 0.5f);
		RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), unsat_check_col, square_sz - pad * 2.0f);
	}

	ImVec2 label_pos = ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y);
	if (g.LogEnabled)
		LogRenderedText(&label_pos, mixed_value ? "[~]" : *vUser ? "[x]" : "[ ]");
	if (label_size.x > 0.0f)
		RenderText(label_pos, label);

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags | ImGuiItemStatusFlags_Checkable | (*vUser ? ImGuiItemStatusFlags_Checked : 0));
	return pressed;
}

BtnIndex_ UI::REGPKeyToBtnIndex(const REGPK_Flag& key)
{
	BtnIndex_ ret = BtnIndex_None;
	switch(key)
	{
	case REGPK_LUp:
		ret = BtnIndex_Xbox_DPadUp;
		break;
	case REGPK_LDown:
		ret = BtnIndex_Xbox_DPadDown;
		break;
	case REGPK_LLeft:
		ret = BtnIndex_Xbox_DPadLeft;
		break;
	case REGPK_LRight:
		ret = BtnIndex_Xbox_DPadRight;
		break;
	case REGPK_RUp:
		ret = BtnIndex_Xbox_Y;
		break;
	case REGPK_RDown:
		ret = BtnIndex_Xbox_A;
		break;
	case REGPK_RLeft:
		ret = BtnIndex_Xbox_X;
		break;
	case REGPK_RRight:
		ret = BtnIndex_Xbox_B;
		break;
	case REGPK_LTrigTop:
		ret = BtnIndex_Xbox_LBumper;
		break;
	case REGPK_LTrigBottom:
		ret = BtnIndex_Xbox_LTrigger;
		break;
	case REGPK_RTrigTop:
		ret = BtnIndex_Xbox_RBumper;
		break;
	case REGPK_RTrigBottom:
		ret = BtnIndex_Xbox_RTrigger;
		break;
	case REGPK_LStickPush:
		ret = BtnIndex_Xbox_LeftStickClick;
		break;
	case REGPK_RStickPush:
		ret = BtnIndex_Xbox_RightStickClick;
		break;
	case REGPK_CLeft:
		ret = BtnIndex_Xbox_Select;
		break;
	case REGPK_CRight:
		ret = BtnIndex_Xbox_Start;
		break;
	}

	return ret;
}

ImVec2 CalcDMC5BtnSize(std::string label, const float& scale = 1.0f, ImVec2 size = { 0, 0 }, float borderSize = 2.0f) {
	auto keyShapeSizeGiven = size;
	auto keyNameSize = ImGui::CalcTextSize(label.c_str(), nullptr, true);
	auto keySizeByName = keyNameSize + ImVec2(borderSize + 3.0f, borderSize + 5.0f) * 2;
	keyShapeSizeGiven = ImVec2(keyShapeSizeGiven.x < keySizeByName.x ? keySizeByName.x : keyShapeSizeGiven.x,
		keyShapeSizeGiven.y < keySizeByName.y ? keySizeByName.y : keyShapeSizeGiven.y);
	auto keyShapeSizeMin = (ImVec2(42.0f, 42.0f) / 1.2f) * scale;

	if (label == "Shift" || label == "LShift" || label == "RShift") {
		keyShapeSizeMin = (ImVec2(108.0f, 42.0f) / 1.2f) * scale;
	}
	else if (label == "Alt" || label == "LAlt" || label == "RAlt"
		|| label == "Ctrl" || label == "LCtrl" || label == "RCtrl"
		|| label == "Tab")
	{
		keyShapeSizeMin = (ImVec2(64.0f, 42.0f) / 1.2f) * scale;
	}
	else if (label == "Enter"
		|| label == "Backspace")
	{
		keyShapeSizeMin = (ImVec2(84.0f, 42.0f) / 1.2f) * scale;
	}
	else if (label == "CapsLock") {
		keyShapeSizeMin = (ImVec2(88.0f, 42.0f) / 1.2f) * scale;
	}

	auto keyShapeSize = ImVec2(keyShapeSizeGiven.x < keyShapeSizeMin.x ? keyShapeSizeMin.x : keyShapeSizeGiven.x,
		keyShapeSizeGiven.y < keyShapeSizeMin.y ? keyShapeSizeMin.y : keyShapeSizeGiven.y);

	return keyShapeSize;
}

ImVec2 CalcDMC5BtnSize(BtnIndex_ index, const float& scale = 1.0f, ImVec2 size = { 0, 0 }) {
	const auto icons = g_framework->get_icons();
	ImRect uvRegion;

	ImVec2 keyShapeSize{ 0.0f, 0.0f };

	switch (index) {
	case 0:
		uvRegion = ImRect(ImVec2(0.000000f, 0.000000f), ImVec2(0.078125f, 0.039063f));
		break;
	case 1:
		uvRegion = ImRect(ImVec2(0.078125f, 0.000000f), ImVec2(0.156250f, 0.039063f));
		break;
	case 2:
		uvRegion = ImRect(ImVec2(0.156250f, 0.000000f), ImVec2(0.234375f, 0.039063f));
		break;
	case 3:
		uvRegion = ImRect(ImVec2(0.234375f, 0.000000f), ImVec2(0.312500f, 0.039063f));
		break;
	case 4:
		uvRegion = ImRect(ImVec2(0.312500f, 0.000000f), ImVec2(0.390625f, 0.039063f));
		break;
	case 5:
		uvRegion = ImRect(ImVec2(0.390625f, 0.000000f), ImVec2(0.468750f, 0.039063f));
		break;
	case 6:
		uvRegion = ImRect(ImVec2(0.000000f, 0.039063f), ImVec2(0.078125f, 0.078125f));
		break;
	case 7:
		uvRegion = ImRect(ImVec2(0.078125f, 0.039063f), ImVec2(0.156250f, 0.078125f));
		break;
	case 8:
		uvRegion = ImRect(ImVec2(0.156250f, 0.039063f), ImVec2(0.234375f, 0.078125f));
		break;
	case 9:
		uvRegion = ImRect(ImVec2(0.234375f, 0.039063f), ImVec2(0.312500f, 0.078125f));
		break;
	case 10:
		uvRegion = ImRect(ImVec2(0.312500f, 0.039063f), ImVec2(0.390625f, 0.078125f));
		break;
	case 11:
		uvRegion = ImRect(ImVec2(0.390625f, 0.039063f), ImVec2(0.468750f, 0.078125f));
		break;
	case 12:
		uvRegion = ImRect(ImVec2(0.000000f, 0.078125f), ImVec2(0.078125f, 0.117188f));
		break;
	case 13:
		uvRegion = ImRect(ImVec2(0.078125f, 0.078125f), ImVec2(0.156250f, 0.117188f));
		break;
	case 14:
		uvRegion = ImRect(ImVec2(0.156250f, 0.078125f), ImVec2(0.234375f, 0.117188f));
		break;
	case 15:
		uvRegion = ImRect(ImVec2(0.234375f, 0.078125f), ImVec2(0.312500f, 0.117188f));
		break;
	case 16:
		uvRegion = ImRect(ImVec2(0.312500f, 0.078125f), ImVec2(0.390625f, 0.117188f));
		break;
	case 17:
		uvRegion = ImRect(ImVec2(0.390625f, 0.078125f), ImVec2(0.468750f, 0.117188f));
		break;
	case 18:
		uvRegion = ImRect(ImVec2(0.000000f, 0.117188f), ImVec2(0.078125f, 0.156250f));
		break;
	case 19:
		uvRegion = ImRect(ImVec2(0.078125f, 0.117188f), ImVec2(0.156250f, 0.156250f));
		break;
	case 20:
		uvRegion = ImRect(ImVec2(0.156250f, 0.117188f), ImVec2(0.234375f, 0.156250f));
		break;
	case 21:
		uvRegion = ImRect(ImVec2(0.234375f, 0.117188f), ImVec2(0.312500f, 0.156250f));
		break;
	case 22:
		uvRegion = ImRect(ImVec2(0.312500f, 0.117188f), ImVec2(0.390625f, 0.156250f));
		break;
	case 23:
		uvRegion = ImRect(ImVec2(0.390625f, 0.117188f), ImVec2(0.468750f, 0.156250f));
		break;
	case 24:
		uvRegion = ImRect(ImVec2(0.000000f, 0.156250f), ImVec2(0.078125f, 0.195313f));
		break;
	case 25:
		uvRegion = ImRect(ImVec2(0.078125f, 0.156250f), ImVec2(0.156250f, 0.195313f));
		break;
	case 26:
		uvRegion = ImRect(ImVec2(0.156250f, 0.156250f), ImVec2(0.234375f, 0.195313f));
		break;
	case 27:
		uvRegion = ImRect(ImVec2(0.234375f, 0.156250f), ImVec2(0.312500f, 0.195313f));
		break;
	case 28:
		uvRegion = ImRect(ImVec2(0.312500f, 0.156250f), ImVec2(0.390625f, 0.195313f));
		break;
	case 29:
		uvRegion = ImRect(ImVec2(0.390625f, 0.156250f), ImVec2(0.468750f, 0.195313f));
		break;
	case 30:
		uvRegion = ImRect(ImVec2(0.000000f, 0.195313f), ImVec2(0.078125f, 0.234375f));
		break;
	case 31:
		uvRegion = ImRect(ImVec2(0.078125f, 0.195313f), ImVec2(0.156250f, 0.234375f));
		break;
	case 32:
		uvRegion = ImRect(ImVec2(0.156250f, 0.195313f), ImVec2(0.234375f, 0.234375f));
		break;
	case 33:
		uvRegion = ImRect(ImVec2(0.234375f, 0.195313f), ImVec2(0.312500f, 0.234375f));
		break;
	case 34:
		uvRegion = ImRect(ImVec2(0.312500f, 0.195313f), ImVec2(0.390625f, 0.234375f));
		break;
	case 35:
		uvRegion = ImRect(ImVec2(0.390625f, 0.195313f), ImVec2(0.468750f, 0.234375f));
		break;
	case 36:
		uvRegion = ImRect(ImVec2(0.468750f, 0.000000f), ImVec2(0.546875f, 0.039063f));
		break;
	case 37:
		uvRegion = ImRect(ImVec2(0.546875f, 0.000000f), ImVec2(0.625000f, 0.039063f));
		break;
	case 38:
		uvRegion = ImRect(ImVec2(0.625000f, 0.000000f), ImVec2(0.703125f, 0.039063f));
		break;
	case 39:
		uvRegion = ImRect(ImVec2(0.703125f, 0.000000f), ImVec2(0.781250f, 0.039063f));
		break;
	case 40:
		uvRegion = ImRect(ImVec2(0.781250f, 0.000000f), ImVec2(0.859375f, 0.039063f));
		break;
	case 41:
		uvRegion = ImRect(ImVec2(0.859375f, 0.000000f), ImVec2(0.937500f, 0.039063f));
		break;
	case 42:
		uvRegion = ImRect(ImVec2(0.468750f, 0.039063f), ImVec2(0.546875f, 0.078125f));
		break;
	case 43:
		uvRegion = ImRect(ImVec2(0.546875f, 0.039063f), ImVec2(0.625000f, 0.078125f));
		break;
	case 44:
		uvRegion = ImRect(ImVec2(0.625000f, 0.039063f), ImVec2(0.703125f, 0.078125f));
		break;
	case 45:
		uvRegion = ImRect(ImVec2(0.703125f, 0.039063f), ImVec2(0.781250f, 0.078125f));
		break;
	case 46:
		uvRegion = ImRect(ImVec2(0.781250f, 0.039063f), ImVec2(0.859375f, 0.078125f));
		break;
	case 47:
		uvRegion = ImRect(ImVec2(0.859375f, 0.039063f), ImVec2(0.937500f, 0.078125f));
		break;
	case 48:
		uvRegion = ImRect(ImVec2(0.468750f, 0.078125f), ImVec2(0.546875f, 0.117188f));
		break;
	case 49:
		uvRegion = ImRect(ImVec2(0.546875f, 0.078125f), ImVec2(0.625000f, 0.117188f));
		break;
	case 50:
		uvRegion = ImRect(ImVec2(0.625000f, 0.078125f), ImVec2(0.703125f, 0.117188f));
		break;
	case 51:
		uvRegion = ImRect(ImVec2(0.703125f, 0.078125f), ImVec2(0.781250f, 0.117188f));
		break;
	case 52:
		uvRegion = ImRect(ImVec2(0.781250f, 0.078125f), ImVec2(0.859375f, 0.117188f));
		break;
	case 53:
		uvRegion = ImRect(ImVec2(0.859375f, 0.078125f), ImVec2(0.937500f, 0.117188f));
		break;
	case 54:
		uvRegion = ImRect(ImVec2(0.468750f, 0.117188f), ImVec2(0.546875f, 0.156250f));
		break;
	case 55:
		uvRegion = ImRect(ImVec2(0.546875f, 0.117188f), ImVec2(0.625000f, 0.156250f));
		break;
	case 56:
		uvRegion = ImRect(ImVec2(0.625000f, 0.117188f), ImVec2(0.703125f, 0.156250f));
		break;
	case 57:
		uvRegion = ImRect(ImVec2(0.703125f, 0.117188f), ImVec2(0.781250f, 0.156250f));
		break;
	case 58:
		uvRegion = ImRect(ImVec2(0.781250f, 0.117188f), ImVec2(0.859375f, 0.156250f));
		break;
	case 59:
		uvRegion = ImRect(ImVec2(0.859375f, 0.117188f), ImVec2(0.937500f, 0.156250f));
		break;
	case 60:
		uvRegion = ImRect(ImVec2(0.468750f, 0.156250f), ImVec2(0.546875f, 0.195313f));
		break;
	case 61:
		uvRegion = ImRect(ImVec2(0.546875f, 0.156250f), ImVec2(0.625000f, 0.195313f));
		break;
	case 62:
		uvRegion = ImRect(ImVec2(0.625000f, 0.156250f), ImVec2(0.703125f, 0.195313f));
		break;
	case 63:
		uvRegion = ImRect(ImVec2(0.703125f, 0.156250f), ImVec2(0.781250f, 0.195313f));
		break;
	case 64:
		uvRegion = ImRect(ImVec2(0.781250f, 0.156250f), ImVec2(0.859375f, 0.195313f));
		break;
	case 65:
		uvRegion = ImRect(ImVec2(0.859375f, 0.156250f), ImVec2(0.937500f, 0.195313f));
		break;
	case 66:
		uvRegion = ImRect(ImVec2(0.468750f, 0.195313f), ImVec2(0.546875f, 0.234375f));
		break;
	case 67:
		uvRegion = ImRect(ImVec2(0.546875f, 0.195313f), ImVec2(0.625000f, 0.234375f));
		break;
	case 68:
		uvRegion = ImRect(ImVec2(0.625000f, 0.195313f), ImVec2(0.703125f, 0.234375f));
		break;
	case 69:
		uvRegion = ImRect(ImVec2(0.703125f, 0.195313f), ImVec2(0.781250f, 0.234375f));
		break;
	case 70:
		uvRegion = ImRect(ImVec2(0.781250f, 0.195313f), ImVec2(0.859375f, 0.234375f));
		break;
	case 71:
		uvRegion = ImRect(ImVec2(0.859375f, 0.195313f), ImVec2(0.937500f, 0.234375f));
		break;
	case 72:
		uvRegion = ImRect(ImVec2(0.000000f, 0.234375f), ImVec2(0.078125f, 0.273438f));
		break;
	case 73:
		uvRegion = ImRect(ImVec2(0.078125f, 0.234375f), ImVec2(0.156250f, 0.273438f));
		break;
	case 74:
		uvRegion = ImRect(ImVec2(0.156250f, 0.234375f), ImVec2(0.234375f, 0.273438f));
		break;
	case 75:
		uvRegion = ImRect(ImVec2(0.234375f, 0.234375f), ImVec2(0.312500f, 0.273438f));
		break;
	case 76:
		uvRegion = ImRect(ImVec2(0.312500f, 0.234375f), ImVec2(0.390625f, 0.273438f));
		break;
	case 77:
		uvRegion = ImRect(ImVec2(0.390625f, 0.234375f), ImVec2(0.468750f, 0.273438f));
		break;
	case 78:
		uvRegion = ImRect(ImVec2(0.468750f, 0.234375f), ImVec2(0.546875f, 0.273438f));
		break;
	case 79:
		uvRegion = ImRect(ImVec2(0.546875f, 0.234375f), ImVec2(0.625000f, 0.273438f));
		break;
	case 80:
		uvRegion = ImRect(ImVec2(0.625000f, 0.234375f), ImVec2(0.703125f, 0.273438f));
		break;
	case 81:
		uvRegion = ImRect(ImVec2(0.703125f, 0.234375f), ImVec2(0.781250f, 0.273438f));
		break;
	case 82:
		uvRegion = ImRect(ImVec2(0.781250f, 0.234375f), ImVec2(0.859375f, 0.273438f));
		break;
	case 83:
		uvRegion = ImRect(ImVec2(0.859375f, 0.234375f), ImVec2(0.937500f, 0.273438f));
		break;
	case 84:
		uvRegion = ImRect(ImVec2(0.000000f, 0.273438f), ImVec2(0.078125f, 0.312500f));
		break;
	case 85:
		uvRegion = ImRect(ImVec2(0.078125f, 0.273438f), ImVec2(0.156250f, 0.312500f));
		break;
	case 86:
		uvRegion = ImRect(ImVec2(0.156250f, 0.273438f), ImVec2(0.234375f, 0.312500f));
		break;
	case 87:
		uvRegion = ImRect(ImVec2(0.234375f, 0.273438f), ImVec2(0.312500f, 0.312500f));
		break;
	case 88:
		uvRegion = ImRect(ImVec2(0.312500f, 0.273438f), ImVec2(0.390625f, 0.312500f));
		break;
	case 89:
		uvRegion = ImRect(ImVec2(0.390625f, 0.273438f), ImVec2(0.468750f, 0.312500f));
		break;
	case 90:
		uvRegion = ImRect(ImVec2(0.468750f, 0.273438f), ImVec2(0.546875f, 0.312500f));
		break;
	case 91:
		uvRegion = ImRect(ImVec2(0.546875f, 0.273438f), ImVec2(0.625000f, 0.312500f));
		break;
	case 92:
		uvRegion = ImRect(ImVec2(0.625000f, 0.273438f), ImVec2(0.703125f, 0.312500f));
		break;
	case 93:
		uvRegion = ImRect(ImVec2(0.703125f, 0.273438f), ImVec2(0.781250f, 0.312500f));
		break;
	case 94:
		uvRegion = ImRect(ImVec2(0.781250f, 0.273438f), ImVec2(0.859375f, 0.312500f));
		break;
	case 95:
		uvRegion = ImRect(ImVec2(0.859375f, 0.273438f), ImVec2(0.937500f, 0.312500f));
		break;
	case 96:
		uvRegion = ImRect(ImVec2(0.000000f, 0.312500f), ImVec2(0.078125f, 0.351562f));
		break;
	case 97:
		uvRegion = ImRect(ImVec2(0.078125f, 0.312500f), ImVec2(0.156250f, 0.351562f));
		break;
	case 98:
		uvRegion = ImRect(ImVec2(0.156250f, 0.312500f), ImVec2(0.234375f, 0.351562f));
		break;
	case 99:
		uvRegion = ImRect(ImVec2(0.234375f, 0.312500f), ImVec2(0.312500f, 0.351562f));
		break;
	case 100:
		uvRegion = ImRect(ImVec2(0.312500f, 0.312500f), ImVec2(0.390625f, 0.351562f));
		break;
	case 101:
		uvRegion = ImRect(ImVec2(0.390625f, 0.312500f), ImVec2(0.468750f, 0.351562f));
		break;
	case 102:
		uvRegion = ImRect(ImVec2(0.468750f, 0.312500f), ImVec2(0.546875f, 0.351562f));
		break;
	case 103:
		uvRegion = ImRect(ImVec2(0.546875f, 0.312500f), ImVec2(0.625000f, 0.351562f));
		break;
	case 104:
		uvRegion = ImRect(ImVec2(0.625000f, 0.312500f), ImVec2(0.703125f, 0.351562f));
		break;
	case 105:
		uvRegion = ImRect(ImVec2(0.703125f, 0.312500f), ImVec2(0.781250f, 0.351562f));
		break;
	case 106:
		uvRegion = ImRect(ImVec2(0.781250f, 0.312500f), ImVec2(0.859375f, 0.351562f));
		break;
	case 107:
		uvRegion = ImRect(ImVec2(0.859375f, 0.312500f), ImVec2(0.937500f, 0.351562f));
		break;
	case 108:
		uvRegion = ImRect(ImVec2(0.000000f, 0.351562f), ImVec2(0.078125f, 0.390625f));
		break;
	case 109:
		uvRegion = ImRect(ImVec2(0.078125f, 0.351562f), ImVec2(0.156250f, 0.390625f));
		break;
	case 110:
		uvRegion = ImRect(ImVec2(0.156250f, 0.351562f), ImVec2(0.234375f, 0.390625f));
		break;
	case 111:
		uvRegion = ImRect(ImVec2(0.234375f, 0.351562f), ImVec2(0.312500f, 0.390625f));
		break;
	case 112:
		uvRegion = ImRect(ImVec2(0.312500f, 0.351562f), ImVec2(0.390625f, 0.390625f));
		break;
	case 113:
		uvRegion = ImRect(ImVec2(0.390625f, 0.351562f), ImVec2(0.468750f, 0.390625f));
		break;
	case 114:
		uvRegion = ImRect(ImVec2(0.468750f, 0.351562f), ImVec2(0.546875f, 0.390625f));
		break;
	case 115:
		uvRegion = ImRect(ImVec2(0.546875f, 0.351562f), ImVec2(0.625000f, 0.390625f));
		break;
	case 116:
		uvRegion = ImRect(ImVec2(0.625000f, 0.351562f), ImVec2(0.703125f, 0.390625f));
		break;
	case 117:
		uvRegion = ImRect(ImVec2(0.703125f, 0.351562f), ImVec2(0.781250f, 0.390625f));
		break;
	case 118:
		uvRegion = ImRect(ImVec2(0.781250f, 0.351562f), ImVec2(0.859375f, 0.390625f));
		break;
	case 119:
		uvRegion = ImRect(ImVec2(0.859375f, 0.351562f), ImVec2(0.937500f, 0.390625f));
		break;
	case 120:
		uvRegion = ImRect(ImVec2(0.000000f, 0.390625f), ImVec2(0.078125f, 0.429688f));
		break;
	case 121:
		uvRegion = ImRect(ImVec2(0.078125f, 0.390625f), ImVec2(0.156250f, 0.429688f));
		break;
	case 122:
		uvRegion = ImRect(ImVec2(0.156250f, 0.390625f), ImVec2(0.234375f, 0.429688f));
		break;
	case 123:
		uvRegion = ImRect(ImVec2(0.234375f, 0.390625f), ImVec2(0.312500f, 0.429688f));
		break;
	case 124:
		uvRegion = ImRect(ImVec2(0.312500f, 0.390625f), ImVec2(0.390625f, 0.429688f));
		break;
	case 125:
		uvRegion = ImRect(ImVec2(0.390625f, 0.390625f), ImVec2(0.468750f, 0.429688f));
		break;
	case 126:
		uvRegion = ImRect(ImVec2(0.468750f, 0.390625f), ImVec2(0.546875f, 0.429688f));
		break;
	case 127:
		uvRegion = ImRect(ImVec2(0.546875f, 0.390625f), ImVec2(0.625000f, 0.429688f));
		break;
	case 128:
		uvRegion = ImRect(ImVec2(0.625000f, 0.390625f), ImVec2(0.703125f, 0.429688f));
		break;
	case 129:
		uvRegion = ImRect(ImVec2(0.703125f, 0.390625f), ImVec2(0.781250f, 0.429688f));
		break;
	case 130:
		uvRegion = ImRect(ImVec2(0.781250f, 0.390625f), ImVec2(0.859375f, 0.429688f));
		break;
	case 131:
		uvRegion = ImRect(ImVec2(0.859375f, 0.390625f), ImVec2(0.937500f, 0.429688f));
		break;
	case 132:
		uvRegion = ImRect(ImVec2(0.000000f, 0.429688f), ImVec2(0.078125f, 0.468750f));
		break;
	case 133:
		uvRegion = ImRect(ImVec2(0.078125f, 0.429688f), ImVec2(0.156250f, 0.468750f));
		break;
	case 134:
		uvRegion = ImRect(ImVec2(0.156250f, 0.429688f), ImVec2(0.234375f, 0.468750f));
		break;
	case 135:
		uvRegion = ImRect(ImVec2(0.234375f, 0.429688f), ImVec2(0.312500f, 0.468750f));
		break;
	case 136:
		uvRegion = ImRect(ImVec2(0.312500f, 0.429688f), ImVec2(0.390625f, 0.468750f));
		break;
	case 137:
		uvRegion = ImRect(ImVec2(0.390625f, 0.429688f), ImVec2(0.468750f, 0.468750f));
		break;
	case 138:
		uvRegion = ImRect(ImVec2(0.468750f, 0.429688f), ImVec2(0.546875f, 0.468750f));
		break;
	case 139:
		uvRegion = ImRect(ImVec2(0.546875f, 0.429688f), ImVec2(0.625000f, 0.468750f));
		break;
	case 140:
		uvRegion = ImRect(ImVec2(0.625000f, 0.429688f), ImVec2(0.703125f, 0.468750f));
		break;
	case 141:
		uvRegion = ImRect(ImVec2(0.703125f, 0.429688f), ImVec2(0.781250f, 0.468750f));
		break;
	case 142:
		uvRegion = ImRect(ImVec2(0.781250f, 0.429688f), ImVec2(0.859375f, 0.468750f));
		break;
	case 143:
		uvRegion = ImRect(ImVec2(0.859375f, 0.429688f), ImVec2(0.937500f, 0.468750f));
		break;
	case 144:
		uvRegion = ImRect(ImVec2(0.000000f, 0.468750f), ImVec2(0.078125f, 0.507813f));
		break;
	case 145:
		uvRegion = ImRect(ImVec2(0.078125f, 0.468750f), ImVec2(0.156250f, 0.507813f));
		break;
	case 146:
		uvRegion = ImRect(ImVec2(0.156250f, 0.468750f), ImVec2(0.234375f, 0.507813f));
		break;
	case 147:
		uvRegion = ImRect(ImVec2(0.234375f, 0.468750f), ImVec2(0.312500f, 0.507813f));
		break;
	case 148:
		uvRegion = ImRect(ImVec2(0.312500f, 0.468750f), ImVec2(0.390625f, 0.507813f));
		break;
	case 149:
		uvRegion = ImRect(ImVec2(0.390625f, 0.468750f), ImVec2(0.468750f, 0.507813f));
		break;
	case 150:
		uvRegion = ImRect(ImVec2(0.468750f, 0.468750f), ImVec2(0.546875f, 0.507813f));
		break;
	case 151:
		uvRegion = ImRect(ImVec2(0.546875f, 0.468750f), ImVec2(0.625000f, 0.507813f));
		break;
	case 152:
		uvRegion = ImRect(ImVec2(0.625000f, 0.468750f), ImVec2(0.703125f, 0.507813f));
		break;
	case 153:
		uvRegion = ImRect(ImVec2(0.703125f, 0.468750f), ImVec2(0.781250f, 0.507813f));
		break;
	case 154:
		uvRegion = ImRect(ImVec2(0.781250f, 0.468750f), ImVec2(0.859375f, 0.507813f));
		break;
	case 155:
		uvRegion = ImRect(ImVec2(0.859375f, 0.468750f), ImVec2(0.937500f, 0.507813f));
		break;
	case 156:
		uvRegion = ImRect(ImVec2(0.000000f, 0.507813f), ImVec2(0.078125f, 0.546875f));
		break;
	case 157:
		uvRegion = ImRect(ImVec2(0.078125f, 0.507813f), ImVec2(0.156250f, 0.546875f));
		break;
	case 158:
		uvRegion = ImRect(ImVec2(0.156250f, 0.507813f), ImVec2(0.234375f, 0.546875f));
		break;
	default:
		uvRegion = ImRect(ImVec2(0, 0), ImVec2(0, 0));
		break;
	}

	if (g_framework->is_dx11()) {
		const auto size = icons.keyIconsDX11.GetSize<float>();
		const auto& width = size.x;
		const auto& height = size.y;

		keyShapeSize = ImVec2(uvRegion.Max.x* width, uvRegion.Max.y* height) - ImVec2(uvRegion.Min.x * width, uvRegion.Min.y * height);
			
	}
	else if (g_framework->is_dx12()) {
		const auto size = icons.keyIconsDX12.GetSize<float>();
		const auto& width = size.x;
		const auto& height = size.y;

		keyShapeSize = ImVec2(uvRegion.Max.x * width, uvRegion.Max.y * height) - ImVec2(uvRegion.Min.x * width, uvRegion.Min.y * height);
	}

	return keyShapeSize;
}

void UI::KeyCaptureWindow(KCWBuffers& kcwBuffers, bool closeBtn /*= false*/)
{
	auto windowCenter = (ImGui::GetMainViewport()->Pos + ImGui::GetMainViewport()->Size / 2);
	auto windowSize = kcwBuffers.windowSizeBuffer + kcwBuffers.windowSizeAddBuffer;
	auto windowPos = windowCenter - windowSize / 2;
	auto& padding = ImGui::GetStyle().FramePadding;

	ImGui::SetNextWindowPos(windowPos);
	ImGui::SetNextWindowSize(windowSize);
	kcwBuffers.windowSizeAddBuffer = ImVec2(0.0f, 0.0f);

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

	ImGui::PushStyleColor(ImGuiCol_Text, 0xFFE7BE4D);

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavFocus
		| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::Begin((kcwBuffers.windowTitle + "##Key Capture Window").c_str(), (closeBtn? &kcwBuffers.drawWindow : nullptr), windowFlags);

	ImGui::PopStyleColor();

	auto& io = ImGui::GetIO();

	ImGuiWindow* window = ImGui::GetCurrentWindow();

	if (!kcwBuffers.needConfirmBuffer) {
		ImGui::InputInt("Key Count", &kcwBuffers.pressedCountBuffer);
		if (kcwBuffers.pressedCountBuffer < 1) {
			kcwBuffers.pressedCountBuffer = 1;
		}
		if (kcwBuffers.pressedCountBuffer > 15) {
			kcwBuffers.pressedCountBuffer = 15;
		}

		ImGui::TextWrapped("Press");
		ImGui::SameLine();
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		DMC5LayoutStyleButton("Backspace", 'M', 0.9);
		ImGui::PopItemFlag();
		ImGui::SameLine();
		ImGui::TextWrapped("to cancel and clear.");
	} else
	{
		auto confirmPrompt = "Are you sure?";
		auto confirmPromptSize = ImGui::CalcTextSize(confirmPrompt);
		ImGui::SetCursorScreenPos({ windowCenter.x - (confirmPromptSize.x + padding.x * 2) / 2,  ImGui::GetCursorScreenPos().y + 20.f });
		ImGui::Text(confirmPrompt);
	}

	const auto& keyboardState = KeyBinder::GetKeyboardState();
	const auto controllerState = KeyBinder::GetControllerKeys(kcwBuffers.gpPressedBuffer = KeyBinder::GPKeyBitToListArray());

	if (kcwBuffers.keyMode == KeyMode_t::Keyboard) {
		if (!kcwBuffers.needConfirmBuffer) {
			for (size_t i = 0; i < keyboardState.size(); i++) {
				if (!io.WantTextInput && keyboardState[i] && i != DIK_BACKSPACE) {
					if (std::find(kcwBuffers.kbPressedBufferList.begin(), kcwBuffers.kbPressedBufferList.end(), i) == kcwBuffers.kbPressedBufferList.end()) {
						kcwBuffers.kbPressedBufferList.push_back(i);
					}
				}
			}

			for (auto it = kcwBuffers.kbPressedBufferList.begin(); it != kcwBuffers.kbPressedBufferList.end();) {
				if (!keyboardState[*it]) {
					it = kcwBuffers.kbPressedBufferList.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
	}
	else
	{
		if (!kcwBuffers.needConfirmBuffer) {
			for (const auto& key : controllerState) {
				if (std::find(kcwBuffers.gpPressedBufferList.begin(), kcwBuffers.gpPressedBufferList.end(), key) == kcwBuffers.gpPressedBufferList.end()) {
					kcwBuffers.gpPressedBufferList.push_back(static_cast<REGPK_Flag>(key));
				}
			}

			for (auto it = kcwBuffers.gpPressedBufferList.begin(); it != kcwBuffers.gpPressedBufferList.end();) {
				if (!(kcwBuffers.gpPressedBuffer & *it)) {
					it = kcwBuffers.gpPressedBufferList.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
	}

	ImVec2 btnsDisplayAreaSize(0, 0);

	auto plusSize = ImGui::CalcTextSize("+");

	std::vector<bool> isNewLine;
	float lastLineMaxWidth = 0.0f;
	float lastLineMaxHeight = 0.0f;
	std::vector<float> buttonRowWidths;
	std::vector<float> buttonRowHeights;

	if (kcwBuffers.keyMode == KeyMode_t::Keyboard) {
		for (auto& key : kcwBuffers.kbPressedBufferList) {
			size_t i = &key - kcwBuffers.kbPressedBufferList.data();

			auto btnSize = CalcDMC5BtnSize(std::string(KeyBinder::DIKToS(key)));
			auto nthButtonWidth = i ? plusSize.x + padding.x * 2 + btnSize.x + padding.x * 2 : btnSize.x;

			if (btnsDisplayAreaSize.x + nthButtonWidth < window->Size.x - 40.0f * g_framework->get_scale()) {
				btnsDisplayAreaSize.x += nthButtonWidth;
				lastLineMaxWidth = btnsDisplayAreaSize.x;
				if (btnsDisplayAreaSize.y < btnSize.y) {
					btnsDisplayAreaSize.y = btnSize.y;
					lastLineMaxHeight = btnsDisplayAreaSize.y;
				}
				isNewLine.push_back(false);
			}
			else {
				buttonRowWidths.push_back(btnsDisplayAreaSize.x);
				buttonRowHeights.push_back(btnsDisplayAreaSize.y);
				isNewLine.push_back(true);
				btnsDisplayAreaSize = ImVec2(0.0f, 0.0f);

				btnsDisplayAreaSize.x += btnSize.x;
				lastLineMaxWidth = btnsDisplayAreaSize.x;
				if (btnsDisplayAreaSize.y < btnSize.y) {
					btnsDisplayAreaSize.y = btnSize.y;
					lastLineMaxHeight = btnsDisplayAreaSize.y;
				}
			}
		}
		buttonRowWidths.push_back(lastLineMaxWidth);
		buttonRowHeights.push_back(lastLineMaxHeight);
	} else
	{
		for (auto& key : kcwBuffers.gpPressedBufferList) {
			size_t i = &key - kcwBuffers.gpPressedBufferList.data();

			auto btnSize = CalcDMC5BtnSize(REGPKeyToBtnIndex(key));
			auto nthButtonWidth = i ? plusSize.x + padding.x * 2 + btnSize.x + padding.x * 2 : btnSize.x;

			if (btnsDisplayAreaSize.x + nthButtonWidth < window->Size.x - 40.0f * g_framework->get_scale()) {
				btnsDisplayAreaSize.x += nthButtonWidth;
				lastLineMaxWidth = btnsDisplayAreaSize.x;
				if (btnsDisplayAreaSize.y < btnSize.y) {
					btnsDisplayAreaSize.y = btnSize.y;
					lastLineMaxHeight = btnsDisplayAreaSize.y;
				}
				isNewLine.push_back(false);
			}
			else {
				buttonRowWidths.push_back(btnsDisplayAreaSize.x);
				buttonRowHeights.push_back(btnsDisplayAreaSize.y);
				isNewLine.push_back(true);
				btnsDisplayAreaSize = ImVec2(0.0f, 0.0f);

				btnsDisplayAreaSize.x += btnSize.x;
				lastLineMaxWidth = btnsDisplayAreaSize.x;
				if (btnsDisplayAreaSize.y < btnSize.y) {
					btnsDisplayAreaSize.y = btnSize.y;
					lastLineMaxHeight = btnsDisplayAreaSize.y;
				}
			}
		}
		buttonRowWidths.push_back(lastLineMaxWidth);
		buttonRowHeights.push_back(lastLineMaxHeight);
	}

	// Get max width
	for (auto& rowWidth : buttonRowWidths) {
		if (btnsDisplayAreaSize.x < rowWidth) {
			btnsDisplayAreaSize.x = rowWidth;
		}
	}

	float plusWidthOffset = 0.0f;

	// Add the size of the + sign after each line to the total width
	if (buttonRowHeights.size() > 1) {
		plusWidthOffset = plusSize.x + padding.x * 2;
		btnsDisplayAreaSize.x += plusWidthOffset;
	}

	// Get total height
	btnsDisplayAreaSize.y = 0.0f;
	for (auto& rowHeight : buttonRowHeights) {
		btnsDisplayAreaSize.y += rowHeight;
	}

	// Add the padding size to the height
	if (buttonRowHeights.size() > 1) {
		btnsDisplayAreaSize.y += (buttonRowHeights.size() - 1) * padding.x * 2;
	}

	auto btnsDrawPosition =	ImVec2(windowCenter.x - buttonRowWidths[0] / 2 - plusWidthOffset / 2, ImGui::GetCursorScreenPos().y + 15.0f/*capWinCenter.y - btnsDisplayAreaSize.y / 2*/);

	auto reservedHeight = btnsDrawPosition.y - window->Pos.y;

	if (reservedHeight + btnsDisplayAreaSize.y > kcwBuffers.windowSizeBuffer.y) {
		auto btnCanvasHeight = reservedHeight + btnsDisplayAreaSize.y - kcwBuffers.windowSizeBuffer.y;

		kcwBuffers.windowSizeAddBuffer = ImVec2(0.0f, btnCanvasHeight + 10.0f);
	}

	ImGui::PushStyleColor(ImGuiCol_Text, 0xFFE7BE4D);

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

	ImGui::SetCursorScreenPos(btnsDrawPosition);

	if (kcwBuffers.keyMode == KeyMode_t::Keyboard) {
		size_t currentLine = 0;
		for (auto& key : kcwBuffers.kbPressedBufferList) {
			if (size_t i = &key - kcwBuffers.kbPressedBufferList.data()) {
				ImGui::SameLine();
				auto posBeforePlus = ImGui::GetCursorScreenPos();
				ImGui::SetCursorScreenPos(ImVec2{ posBeforePlus.x, posBeforePlus.y + (buttonRowHeights[currentLine] / 2 - (plusSize.y + padding.y * 2) / 2) });
				ImGui::Text("+");
				if (isNewLine[i]) {
					currentLine++;
					ImGui::SetCursorScreenPos(ImVec2(windowCenter.x - buttonRowWidths[currentLine] / 2, ImGui::GetCursorScreenPos().y));
				}
				else {
					ImGui::SameLine();
					ImGui::SetCursorScreenPos(ImVec2{ ImGui::GetCursorScreenPos().x, posBeforePlus.y });
				}
			}
			char side = 'M';
			std::string_view keyLabel = KeyBinder::DIKToS(key);

			if (keyLabel == "LAlt" || keyLabel == "LCtrl" ||
				keyLabel == "LShift" || keyLabel == "LAlt" ||
				keyLabel == "LWin") {
				side = 'L';
			}
			else if (keyLabel == "RAlt" || keyLabel == "RCtrl" ||
				keyLabel == "RShift" || keyLabel == "RAlt" ||
				keyLabel == "RWin") {
				side = 'R';
			}

			DMC5LayoutStyleButton(std::string(keyLabel), side);
		}
	}
	else
	{
		size_t currentLine = 0;

		for (const auto& key : kcwBuffers.gpPressedBufferList) {
			if (size_t i = &key - kcwBuffers.gpPressedBufferList.data()) {
				ImGui::SameLine();
				auto posBeforePlus = ImGui::GetCursorScreenPos();
				ImGui::SetCursorScreenPos(ImVec2{ posBeforePlus.x, posBeforePlus.y + (buttonRowHeights[currentLine] / 2 - (plusSize.y + padding.y * 2) / 2) });
				ImGui::Text("+");
				if (isNewLine[i]) {
					currentLine++;
					ImGui::SetCursorScreenPos(ImVec2(windowCenter.x - buttonRowWidths[currentLine] / 2, ImGui::GetCursorScreenPos().y));
				}
				else {
					ImGui::SameLine();
					ImGui::SetCursorScreenPos(ImVec2{ ImGui::GetCursorScreenPos().x, posBeforePlus.y });
				}
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
			DMC5ControlsButton(REGPKeyToBtnIndex(key));
			ImGui::PopStyleVar();
		}
	}

	ImGui::PopItemFlag();

	ImGui::PopStyleColor();

	if (kcwBuffers.needConfirmBuffer) {
		if (kcwBuffers.keyMode == KeyMode_t::Keyboard) {
			const auto& otherBinds = KeyBinder::GetKBBindsOf(kcwBuffers.kbPressedBufferList);

			if (!otherBinds.empty()) {
				auto nodeTitle = "Already in use";
				ImGui::SetCursorScreenPos({ windowCenter.x - (ImGui::CalcTextSize(nodeTitle).x + 8 * padding.x) / 2, ImGui::GetCursorScreenPos().y });
				auto beforePos = ImGui::GetCursorScreenPos();

				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, 0);
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, 0);

				if (ImGui::TreeNode(nodeTitle)) {
					for (const auto& bind : otherBinds)
					{
						ImGui::SetCursorScreenPos({ windowCenter.x - (ImGui::CalcTextSize(bind.c_str()).x + padding.x * 2) / 2, ImGui::GetCursorScreenPos().y });
						ImGui::Text(bind.c_str());
					}

					ImGui::TreePop();
				}

				ImGui::PopStyleColor(2);

				auto afterPos = ImGui::GetCursorScreenPos();

				kcwBuffers.windowSizeAddBuffer.y += afterPos.y - beforePos.y;
			}
		}
		else
		{
			const auto& otherBinds = KeyBinder::GetBindsOf(kcwBuffers.gpPressedBuffer);

			if (!otherBinds.empty()) {
				auto nodeTitle = "Already in use";
				ImGui::SetCursorScreenPos({ windowCenter.x - (ImGui::CalcTextSize(nodeTitle).x + 8 * padding.x) / 2, ImGui::GetCursorScreenPos().y });
				auto beforePos = ImGui::GetCursorScreenPos();

				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, 0);
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, 0);

				if (ImGui::TreeNode(nodeTitle)) {
					for (const auto& bind : otherBinds)
					{
						ImGui::SetCursorScreenPos({ windowCenter.x - (ImGui::CalcTextSize(bind.c_str()).x + padding.x * 2) / 2, ImGui::GetCursorScreenPos().y });
						ImGui::Text(bind.c_str());
					}
				}

				ImGui::PopStyleColor(2);

				auto afterPos = ImGui::GetCursorScreenPos();

				kcwBuffers.windowSizeAddBuffer.y += afterPos.y - beforePos.y;
			}
		}
	}

	// Debug: Show the area the buttons take
	// window->DrawList->AddRectFilled(btnsDrawPosition, btnsDrawPosition + btnsDisplayAreaSize, 0xAAFFFFFF, 3.0f);

	if (!io.WantTextInput && keyboardState[DIK_BACKSPACE]) {
		KeyBinder::SetKBKeys(kcwBuffers.bindName, {});
		KeyBinder::SetGPKeys(kcwBuffers.bindName, REGPK_None);
		g_framework->clear_kc_bind_name();
		kcwBuffers.needConfirmBuffer = false;
		kcwBuffers.drawWindow = false;
	}

	if (kcwBuffers.keyMode == KeyMode_t::Keyboard) {
		if (!kcwBuffers.kbPressedBufferList.empty() && kcwBuffers.kbPressedBufferList.size() == kcwBuffers.pressedCountBuffer) {
			kcwBuffers.needConfirmBuffer = true;
		}
	} else
	{
		if (!kcwBuffers.gpPressedBufferList.empty() && kcwBuffers.gpPressedBufferList.size() == kcwBuffers.pressedCountBuffer) {
			kcwBuffers.needConfirmBuffer = true;
		}
	}

	if(kcwBuffers.needConfirmBuffer)
	{
		auto borderDistance = windowSize.x / 20.0f;
		ImGui::SetCursorScreenPos({ windowPos.x + borderDistance, windowPos.y + windowSize.y - 10.0f - ImGui::CalcTextSize("Yes Remove Other(s) Cancel").y - padding.y * 2});

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);

		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.50f, 0.93f, 0.93f, 0.40f));
		ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.50f, 0.93f, 0.93f, 0.40f));

		ImVec2 buttonSize = {windowSize.x/3.5f, 0.0f};

		if (ImGui::Button("Yes", buttonSize)) {
			if (kcwBuffers.keyMode == KeyMode_t::Keyboard) {
				KeyBinder::SetKBKeys(kcwBuffers.bindName, kcwBuffers.kbPressedBufferList);
			} else
			{
				KeyBinder::SetGPKeys(kcwBuffers.bindName, KeyBinder::GPKeyListArrayToBit(kcwBuffers.gpPressedBufferList));
			}
			g_framework->clear_kc_bind_name();
			kcwBuffers.drawWindow = false;

			kcwBuffers.needConfirmBuffer = false;
		}

		buttonSize = ImGui::GetItemRectSize();

		ImGui::SameLine();
		if (kcwBuffers.keyMode == KeyMode_t::Keyboard) {
			const auto& otherBinds = KeyBinder::GetKBBindsOf(kcwBuffers.kbPressedBufferList);

			if(!otherBinds.empty())
			{
				auto btnLabel = "Remove Other(s)";
				const auto btnLabelRatio = buttonSize.x / (ImGui::CalcTextSize(btnLabel).x + padding.x * 2);

				if(btnLabelRatio < 1.0f)
				{
					ImGui::SetWindowFontScale(btnLabelRatio);
				}

				ImGui::SetCursorScreenPos({ windowCenter.x - buttonSize.x / 2, ImGui::GetCursorScreenPos().y });
				if (ImGui::Button(btnLabel, buttonSize)) {
					KeyBinder::SetKBKeys(kcwBuffers.bindName, kcwBuffers.kbPressedBufferList);
					KeyBinder::RemoveDuplicatesOf(kcwBuffers.kbPressedBufferList, kcwBuffers.bindName);
					g_framework->clear_kc_bind_name();
					kcwBuffers.drawWindow = false;

					kcwBuffers.needConfirmBuffer = false;
				}

				if (btnLabelRatio < 1.0f)
				{
					ImGui::SetWindowFontScale(1.0f);
				}
			}
		} else
		{
			const auto& otherBinds = KeyBinder::GetBindsOf(kcwBuffers.gpPressedBuffer);

			if (!otherBinds.empty())
			{
				auto btnLabel = "Remove Other(s)";
				const auto btnLabelRatio = buttonSize.x / (ImGui::CalcTextSize(btnLabel).x + padding.x * 2);

				if (btnLabelRatio < 1.0f)
				{
					ImGui::SetWindowFontScale(btnLabelRatio);
				}

				ImGui::SetCursorScreenPos({ windowCenter.x - buttonSize.x / 2, ImGui::GetCursorScreenPos().y });
				if (ImGui::Button(btnLabel, buttonSize)) {
					KeyBinder::SetGPKeys(kcwBuffers.bindName, kcwBuffers.gpPressedBuffer);
					KeyBinder::RemoveDuplicatesOf(kcwBuffers.gpPressedBuffer, kcwBuffers.bindName);
					g_framework->clear_kc_bind_name();
					kcwBuffers.drawWindow = false;

					kcwBuffers.needConfirmBuffer = false;
				}

				if (btnLabelRatio < 1.0f)
				{
					ImGui::SetWindowFontScale(1.0f);
				}
			}
		}

		ImGui::SameLine();
		ImGui::SetCursorScreenPos({ windowPos.x + windowSize.x - borderDistance - buttonSize.x, ImGui::GetCursorScreenPos().y });

		if (ImGui::Button("Cancel", buttonSize)) {
			kcwBuffers.needConfirmBuffer = false;
		}

		ImGui::PopStyleColor(2);

		ImGui::PopStyleVar(2);

		kcwBuffers.windowSizeAddBuffer.y += ImGui::GetItemRectSize().y + 10.0f;
	}

	if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenBlockedByPopup)
		&& (io.MouseClicked[0] || io.MouseClicked[1] || io.MouseClicked[2] || io.MouseClicked[3] || io.MouseClicked[4]))
	{
		g_framework->clear_kc_bind_name();
		kcwBuffers.needConfirmBuffer = false;
		kcwBuffers.drawWindow = false;
	}

	ImGui::End();

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(2);
}

void UI::KeyBindButton(const std::string& windowTitle, const std::string& bindName, KCWBuffers& kcwBuffers, const float& scale, bool fitHeight, ImVec4 tint_col, bool showBtnsOnHover)
{
	auto& icons = g_framework->get_icons();

    bool isBtnHovered = false;
    bool isBtnPressed = false;

    if (g_framework->is_dx11()) {
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));

		ImVec2 size = icons.kbIconDX11.GetSize(scale);
		if(fitHeight)
		{
			size = icons.kbIconDX11.GetSize(ImGui::GetItemRectSize().y / size.y);
		}

		float x = ImGui::GetCursorPosX();

        isBtnPressed = ImGui::InvisibleButton(bindName.c_str(), size + ImGui::GetStyle().FramePadding * 2);
		ImGui::SameLine();
		ImGui::SetCursorPosX(x);

		isBtnHovered = ImGui::IsItemHovered();

		if (isBtnHovered) {
			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImGui::GetStyle().FramePadding);
			ImGui::Image(icons.kbIconActiveDX11, size, { 0,0 }, { 1,1 }, tint_col);
		}
		else {
			ImGui::SetCursorPos(ImGui::GetCursorPos() + ImGui::GetStyle().FramePadding);
			ImGui::Image(icons.kbIconDX11, size, { 0,0 }, { 1,1 }, tint_col);
		}

		ImGui::PopStyleColor(4);
    }
    else if (g_framework->is_dx12()) {
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));

        ImVec2 size = icons.kbIconDX12.GetSize(scale);
		if (fitHeight)
		{
			size = icons.kbIconDX12.GetSize(ImGui::GetItemRectSize().y / size.y);
		}
		float x = ImGui::GetCursorPosX();

        isBtnPressed = ImGui::InvisibleButton(bindName.c_str(), size + ImGui::GetStyle().FramePadding * 2);
		ImGui::SameLine();
		ImGui::SetCursorPosX(x);

        isBtnHovered = ImGui::IsItemHovered();

        if (isBtnHovered) {
            ImGui::SetCursorPos(ImGui::GetCursorPos() + ImGui::GetStyle().FramePadding);
            ImGui::Image(icons.kbIconActiveDX12, size, { 0,0 }, { 1,1 }, tint_col);
        }
        else {
            ImGui::SetCursorPos(ImGui::GetCursorPos() + ImGui::GetStyle().FramePadding);
            ImGui::Image(icons.kbIconDX12, size, { 0,0 }, { 1,1 }, tint_col);
        }

        ImGui::PopStyleColor(4);
    }
    else { return; }

	if (isBtnHovered && showBtnsOnHover) {
		std::string kbButtons = "<Not Bound>";

		const auto& rKBKeys = KeyBinder::GetKeyboardKeysStr(bindName);
		if (!rKBKeys.empty()) {
			kbButtons.clear();
			bool first = true;
			for (auto& key : rKBKeys) {
				if (first) {
					kbButtons += key;
					first = false;
				}
				else {
					kbButtons += " + " + key;
				}
			}
		}

		std::string xbButtons = "<Not Bound>";

		const auto& rXboxKeys = KeyBinder::GetXboxConterllerKeysStr(bindName);
		if (!rXboxKeys.empty()) {
			xbButtons.clear();
			bool first = true;
			for (auto& key : rXboxKeys) {
				if (first) {
					xbButtons += key;
					first = false;
				}
				else {
					xbButtons += " + " + key;
				}
			}
		}

		ImGui::BeginTooltip();
		ImGui::Text("KB: %s\nGP: %s", kbButtons.c_str(), xbButtons.c_str());
		ImGui::EndTooltip();
	}

	if (isBtnPressed) {
		if (bindName == g_framework->get_last_kc_bind_name()) {
			g_framework->clear_kc_bind_name();
			kcwBuffers.drawWindow = false;
		}
		else {
			g_framework->set_kcw_name(windowTitle);
			g_framework->set_current_kc_bind_name(bindName);
			kcwBuffers.drawWindow = true;
		}

		kcwBuffers.needConfirmBuffer = false;
	}
}

bool UI::DMC5LayoutStyleButton(std::string label, char side /*= 'M'*/, const float& scale /*= 1.0f*/, ImVec2 size /*= ImVec2(0.0f, 0.0f)*/)
{
    bool ret = false;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
	auto borderSize = 2.0f;

    ImGui::SetWindowFontScale(scale);

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, borderSize);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, 0xFF533B17);
	ImGui::PushStyleColor(ImGuiCol_Border, 0xFFE7BE4D);
	ImGui::PushStyleColor(ImGuiCol_BorderShadow, 0);

    auto keyNameSize = ImGui::CalcTextSize(label.c_str(), nullptr, true);
	auto keyShapeSize = CalcDMC5BtnSize(label, scale, size, borderSize);

	const auto defPos = ImGui::GetCursorScreenPos();
    ret = ImGui::Button("##", keyShapeSize);
    //ret = ImGui::ColorButton("##", ImGui::ColorConvertU32ToFloat4(0xFF533B17), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoDragDrop, keyShapeSize);

	auto middleRect = ImRect(defPos + ImVec2(3.0f + borderSize, borderSize), defPos + keyShapeSize - ImVec2(3.0f + borderSize, 8.0f + borderSize));
	auto middleRectSize = middleRect.Max - middleRect.Min;

	window->DrawList->AddRectFilled(middleRect.Min, middleRect.Max, 0xFF2E1B08, 3.0f);

    ImVec2 labelPos = ImVec2(0.0f, 0.0f);

    switch (side) {
    case 'L':
    case 'l':
    {
        labelPos = ImVec2(middleRect.Min.x + 1.0f, middleRect.Max.y - keyNameSize.y - 1.0f);
    }
        break;

    case 'R':
    case 'r':
    {
        labelPos = ImVec2(middleRect.Max.x - keyNameSize.x - 1.0f, middleRect.Max.y - keyNameSize.y - 1.0f);
    }
        break;

    case 'M':
    case 'm':
    default:
    {
		auto middleRectCenter = (middleRect.Min + middleRect.Max) / 2;
        labelPos =  middleRectCenter - keyNameSize / 2;
    }
        break;
    }

    window->DrawList->AddText(labelPos, 0xFFE7BE4D, label.c_str());

	ImGui::PopStyleColor(3);

	ImGui::PopStyleVar(2);

    ImGui::SetWindowFontScale(1.0f);

    return ret;
}

bool UI::DMC5ControlsButton(BtnIndex_ index, const float& scale)
{
	const auto icons = g_framework->get_icons();
	bool ret = false;
	ImRect uvRegion;

    switch (index) {
	case 0:
		uvRegion = ImRect(ImVec2(0.000000f, 0.000000f), ImVec2(0.078125f, 0.039063f));
		break;
	case 1:
		uvRegion = ImRect(ImVec2(0.078125f, 0.000000f), ImVec2(0.156250f, 0.039063f));
		break;
	case 2:
		uvRegion = ImRect(ImVec2(0.156250f, 0.000000f), ImVec2(0.234375f, 0.039063f));
		break;
	case 3:
		uvRegion = ImRect(ImVec2(0.234375f, 0.000000f), ImVec2(0.312500f, 0.039063f));
		break;
	case 4:
		uvRegion = ImRect(ImVec2(0.312500f, 0.000000f), ImVec2(0.390625f, 0.039063f));
		break;
	case 5:
		uvRegion = ImRect(ImVec2(0.390625f, 0.000000f), ImVec2(0.468750f, 0.039063f));
		break;
	case 6:
		uvRegion = ImRect(ImVec2(0.000000f, 0.039063f), ImVec2(0.078125f, 0.078125f));
		break;
	case 7:
		uvRegion = ImRect(ImVec2(0.078125f, 0.039063f), ImVec2(0.156250f, 0.078125f));
		break;
	case 8:
		uvRegion = ImRect(ImVec2(0.156250f, 0.039063f), ImVec2(0.234375f, 0.078125f));
		break;
	case 9:
		uvRegion = ImRect(ImVec2(0.234375f, 0.039063f), ImVec2(0.312500f, 0.078125f));
		break;
	case 10:
		uvRegion = ImRect(ImVec2(0.312500f, 0.039063f), ImVec2(0.390625f, 0.078125f));
		break;
	case 11:
		uvRegion = ImRect(ImVec2(0.390625f, 0.039063f), ImVec2(0.468750f, 0.078125f));
		break;
	case 12:
		uvRegion = ImRect(ImVec2(0.000000f, 0.078125f), ImVec2(0.078125f, 0.117188f));
		break;
	case 13:
		uvRegion = ImRect(ImVec2(0.078125f, 0.078125f), ImVec2(0.156250f, 0.117188f));
		break;
	case 14:
		uvRegion = ImRect(ImVec2(0.156250f, 0.078125f), ImVec2(0.234375f, 0.117188f));
		break;
	case 15:
		uvRegion = ImRect(ImVec2(0.234375f, 0.078125f), ImVec2(0.312500f, 0.117188f));
		break;
	case 16:
		uvRegion = ImRect(ImVec2(0.312500f, 0.078125f), ImVec2(0.390625f, 0.117188f));
		break;
	case 17:
		uvRegion = ImRect(ImVec2(0.390625f, 0.078125f), ImVec2(0.468750f, 0.117188f));
		break;
	case 18:
		uvRegion = ImRect(ImVec2(0.000000f, 0.117188f), ImVec2(0.078125f, 0.156250f));
		break;
	case 19:
		uvRegion = ImRect(ImVec2(0.078125f, 0.117188f), ImVec2(0.156250f, 0.156250f));
		break;
	case 20:
		uvRegion = ImRect(ImVec2(0.156250f, 0.117188f), ImVec2(0.234375f, 0.156250f));
		break;
	case 21:
		uvRegion = ImRect(ImVec2(0.234375f, 0.117188f), ImVec2(0.312500f, 0.156250f));
		break;
	case 22:
		uvRegion = ImRect(ImVec2(0.312500f, 0.117188f), ImVec2(0.390625f, 0.156250f));
		break;
	case 23:
		uvRegion = ImRect(ImVec2(0.390625f, 0.117188f), ImVec2(0.468750f, 0.156250f));
		break;
	case 24:
		uvRegion = ImRect(ImVec2(0.000000f, 0.156250f), ImVec2(0.078125f, 0.195313f));
		break;
	case 25:
		uvRegion = ImRect(ImVec2(0.078125f, 0.156250f), ImVec2(0.156250f, 0.195313f));
		break;
	case 26:
		uvRegion = ImRect(ImVec2(0.156250f, 0.156250f), ImVec2(0.234375f, 0.195313f));
		break;
	case 27:
		uvRegion = ImRect(ImVec2(0.234375f, 0.156250f), ImVec2(0.312500f, 0.195313f));
		break;
	case 28:
		uvRegion = ImRect(ImVec2(0.312500f, 0.156250f), ImVec2(0.390625f, 0.195313f));
		break;
	case 29:
		uvRegion = ImRect(ImVec2(0.390625f, 0.156250f), ImVec2(0.468750f, 0.195313f));
		break;
	case 30:
		uvRegion = ImRect(ImVec2(0.000000f, 0.195313f), ImVec2(0.078125f, 0.234375f));
		break;
	case 31:
		uvRegion = ImRect(ImVec2(0.078125f, 0.195313f), ImVec2(0.156250f, 0.234375f));
		break;
	case 32:
		uvRegion = ImRect(ImVec2(0.156250f, 0.195313f), ImVec2(0.234375f, 0.234375f));
		break;
	case 33:
		uvRegion = ImRect(ImVec2(0.234375f, 0.195313f), ImVec2(0.312500f, 0.234375f));
		break;
	case 34:
		uvRegion = ImRect(ImVec2(0.312500f, 0.195313f), ImVec2(0.390625f, 0.234375f));
		break;
	case 35:
		uvRegion = ImRect(ImVec2(0.390625f, 0.195313f), ImVec2(0.468750f, 0.234375f));
		break;
	case 36:
		uvRegion = ImRect(ImVec2(0.468750f, 0.000000f), ImVec2(0.546875f, 0.039063f));
		break;
	case 37:
		uvRegion = ImRect(ImVec2(0.546875f, 0.000000f), ImVec2(0.625000f, 0.039063f));
		break;
	case 38:
		uvRegion = ImRect(ImVec2(0.625000f, 0.000000f), ImVec2(0.703125f, 0.039063f));
		break;
	case 39:
		uvRegion = ImRect(ImVec2(0.703125f, 0.000000f), ImVec2(0.781250f, 0.039063f));
		break;
	case 40:
		uvRegion = ImRect(ImVec2(0.781250f, 0.000000f), ImVec2(0.859375f, 0.039063f));
		break;
	case 41:
		uvRegion = ImRect(ImVec2(0.859375f, 0.000000f), ImVec2(0.937500f, 0.039063f));
		break;
	case 42:
		uvRegion = ImRect(ImVec2(0.468750f, 0.039063f), ImVec2(0.546875f, 0.078125f));
		break;
	case 43:
		uvRegion = ImRect(ImVec2(0.546875f, 0.039063f), ImVec2(0.625000f, 0.078125f));
		break;
	case 44:
		uvRegion = ImRect(ImVec2(0.625000f, 0.039063f), ImVec2(0.703125f, 0.078125f));
		break;
	case 45:
		uvRegion = ImRect(ImVec2(0.703125f, 0.039063f), ImVec2(0.781250f, 0.078125f));
		break;
	case 46:
		uvRegion = ImRect(ImVec2(0.781250f, 0.039063f), ImVec2(0.859375f, 0.078125f));
		break;
	case 47:
		uvRegion = ImRect(ImVec2(0.859375f, 0.039063f), ImVec2(0.937500f, 0.078125f));
		break;
	case 48:
		uvRegion = ImRect(ImVec2(0.468750f, 0.078125f), ImVec2(0.546875f, 0.117188f));
		break;
	case 49:
		uvRegion = ImRect(ImVec2(0.546875f, 0.078125f), ImVec2(0.625000f, 0.117188f));
		break;
	case 50:
		uvRegion = ImRect(ImVec2(0.625000f, 0.078125f), ImVec2(0.703125f, 0.117188f));
		break;
	case 51:
		uvRegion = ImRect(ImVec2(0.703125f, 0.078125f), ImVec2(0.781250f, 0.117188f));
		break;
	case 52:
		uvRegion = ImRect(ImVec2(0.781250f, 0.078125f), ImVec2(0.859375f, 0.117188f));
		break;
	case 53:
		uvRegion = ImRect(ImVec2(0.859375f, 0.078125f), ImVec2(0.937500f, 0.117188f));
		break;
	case 54:
		uvRegion = ImRect(ImVec2(0.468750f, 0.117188f), ImVec2(0.546875f, 0.156250f));
		break;
	case 55:
		uvRegion = ImRect(ImVec2(0.546875f, 0.117188f), ImVec2(0.625000f, 0.156250f));
		break;
	case 56:
		uvRegion = ImRect(ImVec2(0.625000f, 0.117188f), ImVec2(0.703125f, 0.156250f));
		break;
	case 57:
		uvRegion = ImRect(ImVec2(0.703125f, 0.117188f), ImVec2(0.781250f, 0.156250f));
		break;
	case 58:
		uvRegion = ImRect(ImVec2(0.781250f, 0.117188f), ImVec2(0.859375f, 0.156250f));
		break;
	case 59:
		uvRegion = ImRect(ImVec2(0.859375f, 0.117188f), ImVec2(0.937500f, 0.156250f));
		break;
	case 60:
		uvRegion = ImRect(ImVec2(0.468750f, 0.156250f), ImVec2(0.546875f, 0.195313f));
		break;
	case 61:
		uvRegion = ImRect(ImVec2(0.546875f, 0.156250f), ImVec2(0.625000f, 0.195313f));
		break;
	case 62:
		uvRegion = ImRect(ImVec2(0.625000f, 0.156250f), ImVec2(0.703125f, 0.195313f));
		break;
	case 63:
		uvRegion = ImRect(ImVec2(0.703125f, 0.156250f), ImVec2(0.781250f, 0.195313f));
		break;
	case 64:
		uvRegion = ImRect(ImVec2(0.781250f, 0.156250f), ImVec2(0.859375f, 0.195313f));
		break;
	case 65:
		uvRegion = ImRect(ImVec2(0.859375f, 0.156250f), ImVec2(0.937500f, 0.195313f));
		break;
	case 66:
		uvRegion = ImRect(ImVec2(0.468750f, 0.195313f), ImVec2(0.546875f, 0.234375f));
		break;
	case 67:
		uvRegion = ImRect(ImVec2(0.546875f, 0.195313f), ImVec2(0.625000f, 0.234375f));
		break;
	case 68:
		uvRegion = ImRect(ImVec2(0.625000f, 0.195313f), ImVec2(0.703125f, 0.234375f));
		break;
	case 69:
		uvRegion = ImRect(ImVec2(0.703125f, 0.195313f), ImVec2(0.781250f, 0.234375f));
		break;
	case 70:
		uvRegion = ImRect(ImVec2(0.781250f, 0.195313f), ImVec2(0.859375f, 0.234375f));
		break;
	case 71:
		uvRegion = ImRect(ImVec2(0.859375f, 0.195313f), ImVec2(0.937500f, 0.234375f));
		break;
	case 72:
		uvRegion = ImRect(ImVec2(0.000000f, 0.234375f), ImVec2(0.078125f, 0.273438f));
		break;
	case 73:
		uvRegion = ImRect(ImVec2(0.078125f, 0.234375f), ImVec2(0.156250f, 0.273438f));
		break;
	case 74:
		uvRegion = ImRect(ImVec2(0.156250f, 0.234375f), ImVec2(0.234375f, 0.273438f));
		break;
	case 75:
		uvRegion = ImRect(ImVec2(0.234375f, 0.234375f), ImVec2(0.312500f, 0.273438f));
		break;
	case 76:
		uvRegion = ImRect(ImVec2(0.312500f, 0.234375f), ImVec2(0.390625f, 0.273438f));
		break;
	case 77:
		uvRegion = ImRect(ImVec2(0.390625f, 0.234375f), ImVec2(0.468750f, 0.273438f));
		break;
	case 78:
		uvRegion = ImRect(ImVec2(0.468750f, 0.234375f), ImVec2(0.546875f, 0.273438f));
		break;
	case 79:
		uvRegion = ImRect(ImVec2(0.546875f, 0.234375f), ImVec2(0.625000f, 0.273438f));
		break;
	case 80:
		uvRegion = ImRect(ImVec2(0.625000f, 0.234375f), ImVec2(0.703125f, 0.273438f));
		break;
	case 81:
		uvRegion = ImRect(ImVec2(0.703125f, 0.234375f), ImVec2(0.781250f, 0.273438f));
		break;
	case 82:
		uvRegion = ImRect(ImVec2(0.781250f, 0.234375f), ImVec2(0.859375f, 0.273438f));
		break;
	case 83:
		uvRegion = ImRect(ImVec2(0.859375f, 0.234375f), ImVec2(0.937500f, 0.273438f));
		break;
	case 84:
		uvRegion = ImRect(ImVec2(0.000000f, 0.273438f), ImVec2(0.078125f, 0.312500f));
		break;
	case 85:
		uvRegion = ImRect(ImVec2(0.078125f, 0.273438f), ImVec2(0.156250f, 0.312500f));
		break;
	case 86:
		uvRegion = ImRect(ImVec2(0.156250f, 0.273438f), ImVec2(0.234375f, 0.312500f));
		break;
	case 87:
		uvRegion = ImRect(ImVec2(0.234375f, 0.273438f), ImVec2(0.312500f, 0.312500f));
		break;
	case 88:
		uvRegion = ImRect(ImVec2(0.312500f, 0.273438f), ImVec2(0.390625f, 0.312500f));
		break;
	case 89:
		uvRegion = ImRect(ImVec2(0.390625f, 0.273438f), ImVec2(0.468750f, 0.312500f));
		break;
	case 90:
		uvRegion = ImRect(ImVec2(0.468750f, 0.273438f), ImVec2(0.546875f, 0.312500f));
		break;
	case 91:
		uvRegion = ImRect(ImVec2(0.546875f, 0.273438f), ImVec2(0.625000f, 0.312500f));
		break;
	case 92:
		uvRegion = ImRect(ImVec2(0.625000f, 0.273438f), ImVec2(0.703125f, 0.312500f));
		break;
	case 93:
		uvRegion = ImRect(ImVec2(0.703125f, 0.273438f), ImVec2(0.781250f, 0.312500f));
		break;
	case 94:
		uvRegion = ImRect(ImVec2(0.781250f, 0.273438f), ImVec2(0.859375f, 0.312500f));
		break;
	case 95:
		uvRegion = ImRect(ImVec2(0.859375f, 0.273438f), ImVec2(0.937500f, 0.312500f));
		break;
	case 96:
		uvRegion = ImRect(ImVec2(0.000000f, 0.312500f), ImVec2(0.078125f, 0.351562f));
		break;
	case 97:
		uvRegion = ImRect(ImVec2(0.078125f, 0.312500f), ImVec2(0.156250f, 0.351562f));
		break;
	case 98:
		uvRegion = ImRect(ImVec2(0.156250f, 0.312500f), ImVec2(0.234375f, 0.351562f));
		break;
	case 99:
		uvRegion = ImRect(ImVec2(0.234375f, 0.312500f), ImVec2(0.312500f, 0.351562f));
		break;
	case 100:
		uvRegion = ImRect(ImVec2(0.312500f, 0.312500f), ImVec2(0.390625f, 0.351562f));
		break;
	case 101:
		uvRegion = ImRect(ImVec2(0.390625f, 0.312500f), ImVec2(0.468750f, 0.351562f));
		break;
	case 102:
		uvRegion = ImRect(ImVec2(0.468750f, 0.312500f), ImVec2(0.546875f, 0.351562f));
		break;
	case 103:
		uvRegion = ImRect(ImVec2(0.546875f, 0.312500f), ImVec2(0.625000f, 0.351562f));
		break;
	case 104:
		uvRegion = ImRect(ImVec2(0.625000f, 0.312500f), ImVec2(0.703125f, 0.351562f));
		break;
	case 105:
		uvRegion = ImRect(ImVec2(0.703125f, 0.312500f), ImVec2(0.781250f, 0.351562f));
		break;
	case 106:
		uvRegion = ImRect(ImVec2(0.781250f, 0.312500f), ImVec2(0.859375f, 0.351562f));
		break;
	case 107:
		uvRegion = ImRect(ImVec2(0.859375f, 0.312500f), ImVec2(0.937500f, 0.351562f));
		break;
	case 108:
		uvRegion = ImRect(ImVec2(0.000000f, 0.351562f), ImVec2(0.078125f, 0.390625f));
		break;
	case 109:
		uvRegion = ImRect(ImVec2(0.078125f, 0.351562f), ImVec2(0.156250f, 0.390625f));
		break;
	case 110:
		uvRegion = ImRect(ImVec2(0.156250f, 0.351562f), ImVec2(0.234375f, 0.390625f));
		break;
	case 111:
		uvRegion = ImRect(ImVec2(0.234375f, 0.351562f), ImVec2(0.312500f, 0.390625f));
		break;
	case 112:
		uvRegion = ImRect(ImVec2(0.312500f, 0.351562f), ImVec2(0.390625f, 0.390625f));
		break;
	case 113:
		uvRegion = ImRect(ImVec2(0.390625f, 0.351562f), ImVec2(0.468750f, 0.390625f));
		break;
	case 114:
		uvRegion = ImRect(ImVec2(0.468750f, 0.351562f), ImVec2(0.546875f, 0.390625f));
		break;
	case 115:
		uvRegion = ImRect(ImVec2(0.546875f, 0.351562f), ImVec2(0.625000f, 0.390625f));
		break;
	case 116:
		uvRegion = ImRect(ImVec2(0.625000f, 0.351562f), ImVec2(0.703125f, 0.390625f));
		break;
	case 117:
		uvRegion = ImRect(ImVec2(0.703125f, 0.351562f), ImVec2(0.781250f, 0.390625f));
		break;
	case 118:
		uvRegion = ImRect(ImVec2(0.781250f, 0.351562f), ImVec2(0.859375f, 0.390625f));
		break;
	case 119:
		uvRegion = ImRect(ImVec2(0.859375f, 0.351562f), ImVec2(0.937500f, 0.390625f));
		break;
	case 120:
		uvRegion = ImRect(ImVec2(0.000000f, 0.390625f), ImVec2(0.078125f, 0.429688f));
		break;
	case 121:
		uvRegion = ImRect(ImVec2(0.078125f, 0.390625f), ImVec2(0.156250f, 0.429688f));
		break;
	case 122:
		uvRegion = ImRect(ImVec2(0.156250f, 0.390625f), ImVec2(0.234375f, 0.429688f));
		break;
	case 123:
		uvRegion = ImRect(ImVec2(0.234375f, 0.390625f), ImVec2(0.312500f, 0.429688f));
		break;
	case 124:
		uvRegion = ImRect(ImVec2(0.312500f, 0.390625f), ImVec2(0.390625f, 0.429688f));
		break;
	case 125:
		uvRegion = ImRect(ImVec2(0.390625f, 0.390625f), ImVec2(0.468750f, 0.429688f));
		break;
	case 126:
		uvRegion = ImRect(ImVec2(0.468750f, 0.390625f), ImVec2(0.546875f, 0.429688f));
		break;
	case 127:
		uvRegion = ImRect(ImVec2(0.546875f, 0.390625f), ImVec2(0.625000f, 0.429688f));
		break;
	case 128:
		uvRegion = ImRect(ImVec2(0.625000f, 0.390625f), ImVec2(0.703125f, 0.429688f));
		break;
	case 129:
		uvRegion = ImRect(ImVec2(0.703125f, 0.390625f), ImVec2(0.781250f, 0.429688f));
		break;
	case 130:
		uvRegion = ImRect(ImVec2(0.781250f, 0.390625f), ImVec2(0.859375f, 0.429688f));
		break;
	case 131:
		uvRegion = ImRect(ImVec2(0.859375f, 0.390625f), ImVec2(0.937500f, 0.429688f));
		break;
	case 132:
		uvRegion = ImRect(ImVec2(0.000000f, 0.429688f), ImVec2(0.078125f, 0.468750f));
		break;
	case 133:
		uvRegion = ImRect(ImVec2(0.078125f, 0.429688f), ImVec2(0.156250f, 0.468750f));
		break;
	case 134:
		uvRegion = ImRect(ImVec2(0.156250f, 0.429688f), ImVec2(0.234375f, 0.468750f));
		break;
	case 135:
		uvRegion = ImRect(ImVec2(0.234375f, 0.429688f), ImVec2(0.312500f, 0.468750f));
		break;
	case 136:
		uvRegion = ImRect(ImVec2(0.312500f, 0.429688f), ImVec2(0.390625f, 0.468750f));
		break;
	case 137:
		uvRegion = ImRect(ImVec2(0.390625f, 0.429688f), ImVec2(0.468750f, 0.468750f));
		break;
	case 138:
		uvRegion = ImRect(ImVec2(0.468750f, 0.429688f), ImVec2(0.546875f, 0.468750f));
		break;
	case 139:
		uvRegion = ImRect(ImVec2(0.546875f, 0.429688f), ImVec2(0.625000f, 0.468750f));
		break;
	case 140:
		uvRegion = ImRect(ImVec2(0.625000f, 0.429688f), ImVec2(0.703125f, 0.468750f));
		break;
	case 141:
		uvRegion = ImRect(ImVec2(0.703125f, 0.429688f), ImVec2(0.781250f, 0.468750f));
		break;
	case 142:
		uvRegion = ImRect(ImVec2(0.781250f, 0.429688f), ImVec2(0.859375f, 0.468750f));
		break;
	case 143:
		uvRegion = ImRect(ImVec2(0.859375f, 0.429688f), ImVec2(0.937500f, 0.468750f));
		break;
	case 144:
		uvRegion = ImRect(ImVec2(0.000000f, 0.468750f), ImVec2(0.078125f, 0.507813f));
		break;
	case 145:
		uvRegion = ImRect(ImVec2(0.078125f, 0.468750f), ImVec2(0.156250f, 0.507813f));
		break;
	case 146:
		uvRegion = ImRect(ImVec2(0.156250f, 0.468750f), ImVec2(0.234375f, 0.507813f));
		break;
	case 147:
		uvRegion = ImRect(ImVec2(0.234375f, 0.468750f), ImVec2(0.312500f, 0.507813f));
		break;
	case 148:
		uvRegion = ImRect(ImVec2(0.312500f, 0.468750f), ImVec2(0.390625f, 0.507813f));
		break;
	case 149:
		uvRegion = ImRect(ImVec2(0.390625f, 0.468750f), ImVec2(0.468750f, 0.507813f));
		break;
	case 150:
		uvRegion = ImRect(ImVec2(0.468750f, 0.468750f), ImVec2(0.546875f, 0.507813f));
		break;
	case 151:
		uvRegion = ImRect(ImVec2(0.546875f, 0.468750f), ImVec2(0.625000f, 0.507813f));
		break;
	case 152:
		uvRegion = ImRect(ImVec2(0.625000f, 0.468750f), ImVec2(0.703125f, 0.507813f));
		break;
	case 153:
		uvRegion = ImRect(ImVec2(0.703125f, 0.468750f), ImVec2(0.781250f, 0.507813f));
		break;
	case 154:
		uvRegion = ImRect(ImVec2(0.781250f, 0.468750f), ImVec2(0.859375f, 0.507813f));
		break;
	case 155:
		uvRegion = ImRect(ImVec2(0.859375f, 0.468750f), ImVec2(0.937500f, 0.507813f));
		break;
	case 156:
		uvRegion = ImRect(ImVec2(0.000000f, 0.507813f), ImVec2(0.078125f, 0.546875f));
		break;
	case 157:
		uvRegion = ImRect(ImVec2(0.078125f, 0.507813f), ImVec2(0.156250f, 0.546875f));
		break;
	case 158:
		uvRegion = ImRect(ImVec2(0.156250f, 0.507813f), ImVec2(0.234375f, 0.546875f));
		break;
	default:
		uvRegion = ImRect(ImVec2(0, 0), ImVec2(0, 0));
		break;
    }

	ImGui::PushStyleColor(ImGuiCol_Button, 0);

	if (g_framework->is_dx11()) {
		const auto size = icons.keyIconsDX11.GetSize<float>();
		const auto& width = size.x;
		const auto& height = size.y;

		ImRect uvRegionAbsolute(
			ImVec2(uvRegion.Min.x * width, uvRegion.Min.y * height),
			ImVec2(uvRegion.Max.x * width, uvRegion.Max.y * height)
		);

		ret = ImGui::ImageButton(icons.keyIconsDX11,
			(uvRegionAbsolute.Max - uvRegionAbsolute.Min) * scale,
			uvRegion.Min, uvRegion.Max);
	}
	else if (g_framework->is_dx12()) {
		const auto size = icons.keyIconsDX12.GetSize<float>();
		const auto& width = size.x;
		const auto& height = size.y;

		ImRect uvRegionAbsolute(
			ImVec2(uvRegion.Min.x * width, uvRegion.Min.y * height),
			ImVec2(uvRegion.Max.x * width, uvRegion.Max.y * height)
		);

		ret = ImGui::ImageButton(icons.keyIconsDX12,
			(uvRegionAbsolute.Max - uvRegionAbsolute.Min) * scale,
			uvRegion.Min, uvRegion.Max);
	}
	else { ret = false; }

	ImGui::PopStyleColor();

	return ret;
}

// Copied straight from CE
template <typename T>
bool UI::Combo(const char* label, const char** items, T count, T& var, T pos, bool save)
{
	bool update = false;
	// ImGui::PushID(GUI_id);
	// GUI_id++;
	ImGui::PushItemWidth(100.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_Border, OUTLINE_GRAY);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.93f, 0.93f, 1.00f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.26f, 0.59f, 0.98f, 0.00f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.05f, 0.11f, 0.20f, 1.00f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.07f, 0.07f, 0.07f, 1.00f));

	// TODO(): Should we use labels?
	if (ImGui::BeginCombo(label, items[var]))
	{
		for (T i = pos; i < count; i++)
		{
			// ImGui::PushID(GUI_id);
			// GUI_id++;
			bool selected = var == i ? true : false;
			if (ImGui::Selectable(items[i], &selected))
			{
				var = i;
				update = true;
			}
			// ImGui::PopID();
			if (selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::PopStyleColor(7);
	ImGui::PopStyleVar(1);
	// ImGui::PopID();
	ImGui::PopItemWidth();
	return update;
}

bool UI::TabBtn(const char* text, bool state, ImVec2 size_arg, float rounding)
{
	bool res;
	float border = state ? 1.2f : 0.0f;
    ImVec4 outline = OUTLINE_NORM;
	ImVec4 background = state ? BUTTON_BC_ACTV : BUTTON_BG_DARK;
	ImVec4 button_text = state ? BUTTON_TEXT_ACT : BUTTON_TEXT_DCT;
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, border);
	ImGui::PushStyleColor(ImGuiCol_Border, outline);
	ImGui::PushStyleColor(ImGuiCol_Button, background);
	ImGui::PushStyleColor(ImGuiCol_Text, button_text);
	res = TopRoundButton(text, size_arg, ImGuiButtonFlags_None);
	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(2);

	return res;
}

 UI::Texture2DDX11::Texture2DDX11(const char* filename, ID3D11Device* pd3dDevice)
     : m_pd3d_device(pd3dDevice)
{
    // Load from disk into a raw RGBA buffer
    m_image_data = stbi_load(filename, &m_width, &m_height, NULL, 4);

    m_is_loaded = Commit();

    stbi_image_free(m_image_data);
}

 UI::Texture2DDX11::Texture2DDX11(const unsigned char* image_data, size_t image_size, ID3D11Device* pd3dDevice)
     : m_pd3d_device(pd3dDevice)
{
    // Load from memory as raw RGBA buffer
    m_image_data = stbi_load_from_memory(image_data, image_size, &m_width, &m_height, NULL, 4);

    m_is_loaded = Commit();

    stbi_image_free(m_image_data);
}

 UI::Texture2DDX11::Texture2DDX11(unsigned char* image_data, int width, int height, ID3D11Device* pd3dDevice)
    : m_image_data(image_data), m_width(width), m_height(height), m_pd3d_device(pd3dDevice)
{
    // Upload the RGBA buffer into VRam
    m_is_loaded = Commit();
}

 bool UI::Texture2DDX11::Commit()
{
	 if (m_image_data == nullptr) {
		 m_last_error = "Invalid image data!";
		 return false;
	 }

	 if (m_pd3d_device == nullptr) {
		 m_last_error = "Invalid device!";
		 return false;
	 }

	// An hresult we will use to make sure everything works
	HRESULT hr = S_OK;

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = m_width;
    desc.Height = m_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    WRL::ComPtr<ID3D11Texture2D> p_texture;
    D3D11_SUBRESOURCE_DATA sub_resource;
    sub_resource.pSysMem = m_image_data;
    sub_resource.SysMemPitch = desc.Width * 4;
    sub_resource.SysMemSlicePitch = 0;
	hr = m_pd3d_device->CreateTexture2D(&desc, &sub_resource, &p_texture);

	if (FAILED(hr)) {
		m_last_error = (std::stringstream{} <<
			"CreateTexture2D() failed!" <<
			" HResult = 0x" <<
			std::hex << std::setfill('0') << std::setw(8) << hr <<
			" -> " << _com_error(hr).ErrorMessage()).str();
		return false;
	}

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
    ZeroMemory(&srv_desc, sizeof(srv_desc));
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = desc.MipLevels;
    srv_desc.Texture2D.MostDetailedMip = 0;
	hr = m_pd3d_device->CreateShaderResourceView(p_texture.Get(), &srv_desc, &m_srv);

	if (FAILED(hr)) {
		m_last_error = (std::stringstream{} <<
			"CreateShaderResourceView() failed!" <<
			" HResult = 0x" <<
			std::hex << std::setfill('0') << std::setw(8) << hr <<
			" -> " << _com_error(hr).ErrorMessage()).str();
		return false;
	}

    return true;
}

 UI::Texture2DDX12::Texture2DDX12(const char* filename, ID3D12Device* pd3dDevice, ID3D12CommandQueue* cmdQueue, ID3D12DescriptorHeap* pd3dSrvDescHeap, UINT descIndexInHeap)
     : m_pd3d_device(pd3dDevice), m_pd3dSrvDescHeap(pd3dSrvDescHeap), m_pd3d_cmd_queue(cmdQueue)
{
    m_handle_increment = m_pd3d_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    
    m_texture_srv_cpu_handle = m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
    m_texture_srv_cpu_handle.ptr += (m_handle_increment * descIndexInHeap);
    
    m_texture_srv_gpu_handle = m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
    m_texture_srv_gpu_handle.ptr += (m_handle_increment * descIndexInHeap);

    // Load from disk into a raw RGBA buffer
    m_image_data = stbi_load(filename, &m_width, &m_height, nullptr, 4);

    m_is_loaded = Commit();

    stbi_image_free(m_image_data);
}

 UI::Texture2DDX12::Texture2DDX12(const unsigned char* image_data, size_t image_size, ID3D12Device* pd3dDevice, ID3D12CommandQueue* cmdQueue, ID3D12DescriptorHeap* pd3dSrvDescHeap, UINT descIndexInHeap)
    : m_pd3d_device(pd3dDevice), m_pd3dSrvDescHeap(pd3dSrvDescHeap), m_pd3d_cmd_queue(cmdQueue)
{
    m_handle_increment = m_pd3d_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    
    m_texture_srv_cpu_handle = m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
    m_texture_srv_cpu_handle.ptr += (m_handle_increment * descIndexInHeap);
    
    m_texture_srv_gpu_handle = m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
    m_texture_srv_gpu_handle.ptr += (m_handle_increment * descIndexInHeap);

    // Load from memory as raw RGBA buffer
    m_image_data = stbi_load_from_memory(image_data, image_size, &m_width, &m_height, nullptr, 4);

    m_is_loaded = Commit();

    stbi_image_free(m_image_data);
}

 UI::Texture2DDX12::Texture2DDX12(unsigned char* image_data, int width, int height, ID3D12Device* pd3dDevice, ID3D12CommandQueue* cmdQueue, ID3D12DescriptorHeap* pd3dSrvDescHeap, UINT descIndexInHeap)
    : m_image_data(image_data), m_width(width), m_height(height), m_pd3d_device(pd3dDevice),
	 m_pd3dSrvDescHeap(pd3dSrvDescHeap), m_pd3d_cmd_queue(cmdQueue)
{
    m_handle_increment = m_pd3d_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    
    m_texture_srv_cpu_handle = m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
    m_texture_srv_cpu_handle.ptr += (m_handle_increment * descIndexInHeap);
    
    m_texture_srv_gpu_handle = m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
    m_texture_srv_gpu_handle.ptr += (m_handle_increment * descIndexInHeap);

    // Upload the RGBA buffer into VRam
    m_is_loaded = Commit();
}

bool UI::Texture2DDX12::Commit()
{
	if (m_image_data == nullptr) {
		m_last_error = "Invalid image data!";
		return false;
	}

	if (m_pd3d_device == nullptr) {
		m_last_error = "Invalid device!";
		return false;
	}

	if (m_pd3d_cmd_queue == nullptr) {
		m_last_error = "Invalid command queue!";
		return false;
	}

	// An hresult we will use to make sure everything works
	HRESULT hr = S_OK;

    // Create texture resource
    D3D12_HEAP_PROPERTIES props;
    memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
    props.Type = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = m_width;
    desc.Height = m_height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	hr = m_pd3d_device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_tex_resource));

	if (FAILED(hr)) {
		m_last_error = (std::stringstream{} <<
			"CreateCommittedResource() for main texture upload buffer failed!" <<
			" HResult = 0x" <<
			std::hex << std::setfill('0') << std::setw(8) << hr <<
			" -> " << _com_error(hr).ErrorMessage()).str();
		return false;
	}

    // Create a temporary upload resource to move the data in
    UINT upload_pitch = (m_width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
    UINT upload_size = m_height * upload_pitch;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = upload_size;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    props.Type = D3D12_HEAP_TYPE_UPLOAD;
    props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    WRL::ComPtr<ID3D12Resource> upload_buffer;
    hr = m_pd3d_device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload_buffer));

	if (FAILED(hr)) {
		m_last_error = (std::stringstream{} << 
			"CreateCommittedResource() for temporary texture upload buffer failed!" <<
			" HResult = 0x" << 
			std::hex << std::setfill('0') << std::setw(8) << hr <<
			" -> " << _com_error(hr).ErrorMessage()).str();
		return false;
	}

    // Write pixels into the upload resource
    void* mapped = nullptr;
    D3D12_RANGE range = { 0, upload_size };
    hr = upload_buffer->Map(0, &range, &mapped);

	if (FAILED(hr)) {
		m_last_error = (std::stringstream{} <<
			"Map() for upload buffer failed!" <<
			" HResult = 0x" <<
			std::hex << std::setfill('0') << std::setw(8) << hr <<
			" -> " << _com_error(hr).ErrorMessage()).str();
		return false;
	}

    for (int y = 0; y < m_height; y++)
        memcpy((void*)((uintptr_t)mapped + y * upload_pitch), m_image_data + y * m_width * 4, m_width * 4);
    upload_buffer->Unmap(0, &range);

    // Copy the upload resource content into the real resource
    D3D12_TEXTURE_COPY_LOCATION src_location = {};
    src_location.pResource = upload_buffer.Get();
    src_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src_location.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    src_location.PlacedFootprint.Footprint.Width = m_width;
    src_location.PlacedFootprint.Footprint.Height = m_height;
    src_location.PlacedFootprint.Footprint.Depth = 1;
    src_location.PlacedFootprint.Footprint.RowPitch = upload_pitch;

    D3D12_TEXTURE_COPY_LOCATION dst_location = {};
    dst_location.pResource = m_tex_resource.Get();
    dst_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst_location.SubresourceIndex = 0;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_tex_resource.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    // Create a temporary command queue to do the copy with
    WRL::ComPtr<ID3D12Fence> fence;
    hr = m_pd3d_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	if (FAILED(hr)) {
		m_last_error = (std::stringstream{} <<
			"CreateFence() failed!" <<
			" HResult = 0x" <<
			std::hex << std::setfill('0') << std::setw(8) << hr <<
			" -> " << _com_error(hr).ErrorMessage()).str();
		return false;
	}

    HANDLE event = CreateEvent(0, 0, 0, 0);

    D3D12_COMMAND_QUEUE_DESC queue_desc = {};
    queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.NodeMask = 1;

    WRL::ComPtr<ID3D12CommandAllocator> cmd_alloc;
    hr = m_pd3d_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmd_alloc));

	if (FAILED(hr)) {
		m_last_error = (std::stringstream{} <<
			"CreateCommandAllocator() failed!" <<
			" HResult = 0x" <<
			std::hex << std::setfill('0') << std::setw(8) << hr <<
			" -> " << _com_error(hr).ErrorMessage()).str();
		return false;
	}

    WRL::ComPtr<ID3D12GraphicsCommandList> cmd_list;
    hr = m_pd3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmd_alloc.Get(), nullptr, IID_PPV_ARGS(&cmd_list));

	if (FAILED(hr)) {
		m_last_error = (std::stringstream{} <<
			"CreateCommandList() failed!" <<
			" HResult = 0x" <<
			std::hex << std::setfill('0') << std::setw(8) << hr <<
			" -> " << _com_error(hr).ErrorMessage()).str();
		return false;
	}

    cmd_list->CopyTextureRegion(&dst_location, 0, 0, 0, &src_location, nullptr);
    cmd_list->ResourceBarrier(1, &barrier);

    hr = cmd_list->Close();

	if (FAILED(hr)) {
		m_last_error = (std::stringstream{} <<
			"Close() failed for cmd_list!" <<
			" HResult = 0x" <<
			std::hex << std::setfill('0') << std::setw(8) << hr <<
			" -> " << _com_error(hr).ErrorMessage()).str();
		return false;
	}

    // Execute the copy
    m_pd3d_cmd_queue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(cmd_list.GetAddressOf()));
    hr = m_pd3d_cmd_queue->Signal(fence.Get(), 1);

	if (FAILED(hr)) {
		m_last_error = (std::stringstream{} <<
			"Signal() failed for cmd_queue!" <<
			" HResult = 0x" <<
			std::hex << std::setfill('0') << std::setw(8) << hr <<
			" -> " << _com_error(hr).ErrorMessage()).str();
		return false;
	}

    // Wait for everything to complete
	hr = fence->SetEventOnCompletion(1, event);

	if (FAILED(hr)) {
		m_last_error = (std::stringstream{} <<
			"SetEventOnCompletion() failed for fence!" <<
			" HResult = 0x" <<
			std::hex << std::setfill('0') << std::setw(8) << hr <<
			" -> " << _com_error(hr).ErrorMessage()).str();
		return false;
	}

    WaitForSingleObject(event, INFINITE);

    // Tear down our temporary command queue and release the upload resource
    CloseHandle(event);

    // Create a shader resource view for the texture
    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;
    ZeroMemory(&srv_desc, sizeof(srv_desc));
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = desc.MipLevels;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    m_pd3d_device->CreateShaderResourceView(m_tex_resource.Get(), &srv_desc, m_texture_srv_cpu_handle);
	
    // Return results
    return true;
}
