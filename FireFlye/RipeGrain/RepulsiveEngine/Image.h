#pragma once
#include "Font.h"
#include<memory>
#include<filesystem>
#include<type_traits>
#include<wrl.h>
#include<span>
#include"GDIPlusManager.h"

struct ColorType
{
	unsigned char b = 255;
	unsigned char g = 255;
	unsigned char r = 255;
	unsigned char a = 255;
};

class Image
{
private:
	static GDIPlusManager manager;
	std::unique_ptr<Gdiplus::Bitmap> bitmap;
public:
	Image(const std::filesystem::path& file);
	Image(unsigned int width, unsigned height);
	Image(std::span<char> source);
	Image(const Image& img);
	unsigned int GetHeight() const;
	unsigned int GetWidth() const;
	ColorType GetPixel(unsigned int x , unsigned int y) const;
	void SetPixel(unsigned int x, unsigned int y, ColorType color);
	void DrawLine(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, ColorType color);
	void DrawString(const std::wstring& text, ColorType color, unsigned int x, unsigned int y, Font& font);
	void DrawImage(const Image& img , unsigned int x, unsigned int y);
	void Clear(ColorType color = {});
	ColorType* Raw();
	const ColorType* Raw() const;
public:
	void Resize(int width , int height);
public:
	Image& operator=(const Image& img);
private:
	void static get_decoder_from_ext(const std::string& ext, CLSID& Clsid);
public:
	std::vector<char> SaveToBuffer(const std::string ext_type) const;
	void Save(const std::filesystem::path& file) const;
};