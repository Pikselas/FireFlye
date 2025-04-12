#pragma once
#include <memory>
#include <filesystem>
#include "GDIPlusManager.h"

class FontFamily
{
	friend class Font;
private:
	Gdiplus::FontFamily family;
public:
	FontFamily(const std::filesystem::path& file)
	{
		Gdiplus::PrivateFontCollection fnt;
		auto stat = fnt.AddFontFile(file.wstring().c_str());
		if (stat != Gdiplus::Status::Ok)
		{
			throw std::runtime_error("error loading file");
		}
		int nums = 1;
		stat = fnt.GetFamilies(nums, &family, &nums);
	}
};

class Font
{
	friend class Image;
private:
	std::shared_ptr<Gdiplus::Font> font;
public:
	Font() = default;
	Font(FontFamily& family, unsigned int font_size) : font(std::make_shared<Gdiplus::Font>(&family.family, font_size, Gdiplus::FontStyle::FontStyleRegular, Gdiplus::UnitPixel))
	{}
public:
	std::pair<unsigned int, unsigned int> CalculateTextSize(const std::wstring& txt) const
	{
		Gdiplus::RectF layoutRect(0.0f, 0.0f, 1500.0f, 1500.0f);
		Gdiplus::RectF boundingBox;
		auto hdc = GetDC(nullptr);
		Gdiplus::Graphics graphics(hdc);
		graphics.MeasureString(txt.c_str(), txt.length(), font.get(), layoutRect, &boundingBox);
		ReleaseDC(nullptr, hdc);

		return { boundingBox.Width , boundingBox.Height };
	}
};