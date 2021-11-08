#include "ui.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize.h"

#include <imgui/imgui_internal.h>

#include "fw-imgui/imgui_impl_dx11.h"
#include "fw-imgui/imgui_impl_dx12.h"
#include "fw-imgui/imgui_impl_win32.h"

// clang-format off

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

static inline ImVec2 operator*(const ImVec2& lhs, const float rhs)              { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
static inline ImVec2 operator/(const ImVec2& lhs, const float rhs)              { return ImVec2(lhs.x / rhs, lhs.y / rhs); }
static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
static inline ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
static inline ImVec2 operator/(const ImVec2& lhs, const ImVec2& rhs)            { return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y); }
static inline ImVec2& operator*=(ImVec2& lhs, const float rhs)                  { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
static inline ImVec2& operator/=(ImVec2& lhs, const float rhs)                  { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
static inline ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
static inline ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
static inline ImVec2& operator*=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
static inline ImVec2& operator/=(ImVec2& lhs, const ImVec2& rhs)                { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }
static inline ImVec4 operator+(const ImVec4& lhs, const ImVec4& rhs)            { return ImVec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
static inline ImVec4 operator-(const ImVec4& lhs, const ImVec4& rhs)            { return ImVec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
static inline ImVec4 operator*(const ImVec4& lhs, const ImVec4& rhs)            { return ImVec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }

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
		DataTypeApplyOpFromText(data_buf, g.InputTextState.InitialTextA.Data, data_type, p_data, NULL);
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
    ImGui::Text("Value: %.3f", *v);
	ImGui::PopStyleVar(2);
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
     : m_pd3dDevice(pd3dDevice)
{
    // Load from disk into a raw RGBA buffer
    m_image_data = stbi_load(filename, &m_width, &m_height, NULL, 4);

    m_is_loaded = Commit();

    stbi_image_free(m_image_data);
}

 UI::Texture2DDX11::Texture2DDX11(const unsigned char* image_data, size_t image_size, ID3D11Device* pd3dDevice)
     : m_pd3dDevice(pd3dDevice)
{
    // Load from memory as raw RGBA buffer
    m_image_data = stbi_load_from_memory(image_data, image_size, &m_width, &m_height, NULL, 4);

    m_is_loaded = Commit();

    stbi_image_free(m_image_data);
}

 UI::Texture2DDX11::Texture2DDX11(unsigned char* image_data, int width, int height, ID3D11Device* pd3dDevice)
    : m_image_data(image_data), m_width(width), m_height(height), m_pd3dDevice(pd3dDevice)
{
    // Upload the RGBA buffer into VRam
    m_is_loaded = Commit();
}

 inline bool UI::Texture2DDX11::Commit()
{
    if (m_image_data == NULL)
        return false;

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

    ID3D11Texture2D *pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = m_image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    m_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    m_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &m_srv);
    pTexture->Release();

    return true;
}

 UI::Texture2DDX12::Texture2DDX12(const char* filename, ID3D12Device* pd3dDevice, ID3D12DescriptorHeap* pd3dSrvDescHeap)
     : m_pd3dDevice(pd3dDevice), m_pd3dSrvDescHeap(pd3dSrvDescHeap)
{
    m_handle_increment = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_srv_cpu_handle = m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
    m_srv_cpu_handle.ptr += (m_handle_increment * m_descriptor_index);
    m_texture_srv_gpu_handle = m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
    m_texture_srv_gpu_handle.ptr += (m_handle_increment * m_descriptor_index);

    // Load from disk into a raw RGBA buffer
    m_image_data = stbi_load(filename, &m_width, &m_height, NULL, 4);

    m_is_loaded = Commit();

    stbi_image_free(m_image_data);
}

 UI::Texture2DDX12::Texture2DDX12(const unsigned char* image_data, size_t image_size, ID3D12Device* pd3dDevice, ID3D12DescriptorHeap* pd3dSrvDescHeap)
    : m_pd3dDevice(pd3dDevice), m_pd3dSrvDescHeap(pd3dSrvDescHeap)
{
    m_handle_increment = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_srv_cpu_handle = m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
    m_srv_cpu_handle.ptr += (m_handle_increment * m_descriptor_index);
    m_texture_srv_gpu_handle = m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
    m_texture_srv_gpu_handle.ptr += (m_handle_increment * m_descriptor_index);

    // Load from memory as raw RGBA buffer
    m_image_data = stbi_load_from_memory(image_data, image_size, &m_width, &m_height, NULL, 4);

    m_is_loaded = Commit();

    stbi_image_free(m_image_data);
}

 UI::Texture2DDX12::Texture2DDX12(unsigned char* image_data, int width, int height, ID3D12Device* pd3dDevice, ID3D12DescriptorHeap* pd3dSrvDescHeap)
    : m_image_data(image_data), m_width(width), m_height(height), m_pd3dDevice(pd3dDevice), m_pd3dSrvDescHeap(pd3dSrvDescHeap)
{
    m_handle_increment = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_srv_cpu_handle = m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
    m_srv_cpu_handle.ptr += (m_handle_increment * m_descriptor_index);
    m_texture_srv_gpu_handle = m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
    m_texture_srv_gpu_handle.ptr += (m_handle_increment * m_descriptor_index);

    // Upload the RGBA buffer into VRam
    m_is_loaded = Commit();
}

 inline bool UI::Texture2DDX12::Commit()
{
    if (m_image_data == NULL)
        return false;

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

    ID3D12Resource* pTexture = NULL;
    m_pd3dDevice->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&pTexture));

    // Create a temporary upload resource to move the data in
    UINT uploadPitch = (m_width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
    UINT uploadSize = m_height * uploadPitch;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = uploadSize;
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

    ID3D12Resource* uploadBuffer = NULL;
    HRESULT hr = m_pd3dDevice->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&uploadBuffer));
    IM_ASSERT(SUCCEEDED(hr));

    // Write pixels into the upload resource
    void* mapped = NULL;
    D3D12_RANGE range = { 0, uploadSize };
    hr = uploadBuffer->Map(0, &range, &mapped);
    IM_ASSERT(SUCCEEDED(hr));
    for (int y = 0; y < m_height; y++)
        memcpy((void*)((uintptr_t)mapped + y * uploadPitch), m_image_data + y * m_width * 4, m_width * 4);
    uploadBuffer->Unmap(0, &range);

    // Copy the upload resource content into the real resource
    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = uploadBuffer;
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srcLocation.PlacedFootprint.Footprint.Width = m_width;
    srcLocation.PlacedFootprint.Footprint.Height = m_height;
    srcLocation.PlacedFootprint.Footprint.Depth = 1;
    srcLocation.PlacedFootprint.Footprint.RowPitch = uploadPitch;

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = pTexture;
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = 0;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = pTexture;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    // Create a temporary command queue to do the copy with
    ID3D12Fence* fence = NULL;
    hr = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    IM_ASSERT(SUCCEEDED(hr));

    HANDLE event = CreateEvent(0, 0, 0, 0);
    IM_ASSERT(event != NULL);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 1;

    ID3D12CommandQueue* cmdQueue = NULL;
    hr = m_pd3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue));
    IM_ASSERT(SUCCEEDED(hr));

    ID3D12CommandAllocator* cmdAlloc = NULL;
    hr = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
    IM_ASSERT(SUCCEEDED(hr));

    ID3D12GraphicsCommandList* cmdList = NULL;
    hr = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc, NULL, IID_PPV_ARGS(&cmdList));
    IM_ASSERT(SUCCEEDED(hr));

    cmdList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, NULL);
    cmdList->ResourceBarrier(1, &barrier);

    hr = cmdList->Close();
    IM_ASSERT(SUCCEEDED(hr));

    // Execute the copy
    cmdQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&cmdList);
    hr = cmdQueue->Signal(fence, 1);
    IM_ASSERT(SUCCEEDED(hr));

    // Wait for everything to complete
    fence->SetEventOnCompletion(1, event);
    WaitForSingleObject(event, INFINITE);

    // Tear down our temporary command queue and release the upload resource
    cmdList->Release();
    cmdAlloc->Release();
    cmdQueue->Release();
    CloseHandle(event);
    fence->Release();
    uploadBuffer->Release();

    // Create a shader resource view for the texture
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    m_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, m_srv_cpu_handle);

    // Return results
    m_tex_resource = pTexture;
    
    return true;
}
