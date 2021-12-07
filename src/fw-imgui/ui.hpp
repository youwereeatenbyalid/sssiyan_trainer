#pragma once
// clang-format off
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi")

#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#include <imgui/imgui.h>

#define OUTLINE_GRAY		ImVec4(0.29f, 0.29f, 0.29f, 1.00f)
#define OUTLINE_NORM		ImVec4(0.50f, 0.93f, 0.93f, 1.00f)
#define ELEMENT_BG_DARK		ImVec4(0.07f, 0.07f, 0.07f, 1.00f)
#define ELEMENT_BC_ACTV		ImVec4(0.07f, 0.22f, 0.29f, 1.00f)
#define ELEMENT_TEXT_DCT	ImVec4(0.09f, 0.60f, 0.64f, 1.00f)
#define ELEMENT_TEXT_ACT	ImVec4(0.49f, 0.91f, 0.91f, 1.00f)
#define ELEMENT_STYLE_ACT	ImVec4(0.09f, 0.25f, 0.33f, 1.00f)
#define ELEMENT_STYLE_DCT	ImVec4(0.07f, 0.07f, 0.07f, 1.00f)
#define ELEMENT_LABEL_ACT	ImVec4(0.95f, 0.95f, 0.95f, 1.00f)
#define ELEMENT_LABEL_DCT	ImVec4(0.20f, 0.20f, 0.20f, 1.00f)
#define BUTTON_BG_DARK		ImVec4(0.07f, 0.07f, 0.07f, 1.00f)
#define BUTTON_BC_ACTV		ImVec4(0.07f, 0.22f, 0.29f, 1.00f)
#define BUTTON_TEXT_DCT		ImVec4(0.09f, 0.60f, 0.64f, 1.00f)
#define BUTTON_TEXT_ACT		ImVec4(0.49f, 0.91f, 0.91f, 1.00f)

namespace UI {
	unsigned int stb_decompress_length(const unsigned char *input);
	unsigned int stb_decompress(unsigned char *output, const unsigned char *i, unsigned int length);
	int stb_resize(const unsigned char *input_pixels , int input_w , int input_h , int input_stride_in_bytes,
                   unsigned char *output_pixels, int output_w, int output_h, int output_stride_in_bytes,
                   int num_channels);

	void RenderRoundTopFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding);
	bool TopRoundButton(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags);

	bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", float thickness = 1.0f, ImGuiSliderFlags flags = 0);
	bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", float thickness = 1.0f, ImGuiSliderFlags flags = 0);

	// Copied straight from CE
	template<typename T>
	bool Combo(const char* label, const char** items, T count, T& var, T pos, bool save);

	bool TabBtn(const char* text, bool state, ImVec2 size_arg = ImVec2(0.0f, 0.0f), float rounding = 0.0f);

	class Texture2DDX11 {
	public:
		Texture2DDX11() = default;

		Texture2DDX11(const char* filename, ID3D11Device* pd3dDevice);
		Texture2DDX11(const unsigned char* image_data, size_t image_size, ID3D11Device* pd3dDevice);
		Texture2DDX11(unsigned char* image_data, int width, int height, ID3D11Device* pd3dDevice);

		unsigned char* GetImageData() const { return m_image_data; }
		ID3D11ShaderResourceView* GetTexture() const { return m_srv; }
		int GetWidth() const { return m_width; }
		int GetHeight() const { return m_height; }
		bool IsValid() const { return m_is_loaded; }

		inline operator void*() { return (void*)GetTexture(); }
		inline operator ID3D11ShaderResourceView*() { return GetTexture(); }

	private:
		inline bool Commit();

	private:
		unsigned char* m_image_data{nullptr};

		ID3D11Device* m_pd3dDevice{nullptr};
		ID3D11ShaderResourceView* m_srv{nullptr};
		int m_width{NULL};
		int m_height{NULL};

		bool m_is_loaded{false};
	};

	class Texture2DDX12 {
	public:
		Texture2DDX12() = default;
		Texture2DDX12(const char* filename, ID3D12Device* pd3dDevice, ID3D12DescriptorHeap* pd3dSrvDescHeap);
		Texture2DDX12(const unsigned char* image_data, size_t image_size, ID3D12Device* pd3dDevice, ID3D12DescriptorHeap* pd3dSrvDescHeap);
		Texture2DDX12(unsigned char* image_data, int width, int height, ID3D12Device* pd3dDevice, ID3D12DescriptorHeap* pd3dSrvDescHeap);

		UINT64 GetTexture() const { return m_texture_srv_gpu_handle.ptr; }
		int GetWidth() const { return m_width; }
		int GetHeight() const { return m_height; }
		bool IsValid() const { return m_is_loaded; }

		inline operator void*() { return (void*)GetTexture(); }
		inline operator UINT64() { return GetTexture(); }

	private:
		inline bool Commit();

	private:
		unsigned char* m_image_data{nullptr};

		ID3D12Resource* m_tex_resource{nullptr};
		ID3D12Device* m_pd3dDevice{nullptr};
		ID3D12DescriptorHeap* m_pd3dSrvDescHeap{nullptr};

		UINT m_handle_increment;
		D3D12_CPU_DESCRIPTOR_HANDLE m_srv_cpu_handle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_texture_srv_gpu_handle;
		int m_descriptor_index = 2;
		
		int m_width{NULL};
		int m_height{NULL};

		bool m_is_loaded{false};
	};
}