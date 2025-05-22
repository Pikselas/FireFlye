#include "Image.h"

GDIPlusManager Image::manager;

Image::Image(const std::filesystem::path& file) : bitmap(std::make_unique<Gdiplus::Bitmap>(file.c_str()))
{
	if (bitmap->GetLastStatus() != Gdiplus::Status::Ok)
	{
		throw std::runtime_error("Failed to load image");
	}
}

Image::Image(unsigned int width,unsigned int height) : bitmap(std::make_unique<Gdiplus::Bitmap>(width,height,PixelFormat32bppARGB))
{}

Image::Image(std::span<char> source)
{
	Microsoft::WRL::ComPtr<IStream> stream;
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, source.size());
	if (!hGlobal) 
	{
		throw std::runtime_error("failed to creat global");
	}

	void* pBuffer = GlobalLock(hGlobal);
	if (!pBuffer) 
	{
		GlobalFree(hGlobal);
		throw std::runtime_error("failed to lock global");
	}
	memcpy(pBuffer, source.data(), source.size());
	GlobalUnlock(hGlobal);

	if (FAILED(CreateStreamOnHGlobal(hGlobal, TRUE, &stream))) 
	{
		GlobalFree(hGlobal);
		throw std::runtime_error("failed to create stream on global");
	}
	bitmap = std::make_unique<Gdiplus::Bitmap>(stream.Get());
}

Image::Image(const Image& img) : Image(img.GetWidth(), img.GetHeight())
{
	*this = img;
}

unsigned int Image::GetHeight() const
{
	return bitmap->GetHeight();
}

unsigned int Image::GetWidth() const 
{
    return bitmap->GetWidth();
}

ColorType Image::GetPixel(unsigned int x, unsigned int y) const
{
	Gdiplus::Color c;
	auto bitmapPtr = const_cast<Gdiplus::Bitmap*>(bitmap.get());
	if (bitmapPtr->GetPixel(x, y, &c) != Gdiplus::Ok)
	{
		return { 0,0,0,0 };
	}
	return ColorType{ c.GetBlue(),c.GetGreen(),c.GetRed(),c.GetAlpha()};
	//return static_cast<ColorType>(c.GetValue());
}

void Image::SetPixel(unsigned int x, unsigned int y, ColorType color)
{
	bitmap->SetPixel(x, y, Gdiplus::Color(color.a, color.r, color.g, color.b));
}

void Image::DrawLine(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, ColorType color)
{
	Gdiplus::Graphics graphics(bitmap.get());
	Gdiplus::Pen pen(Gdiplus::Color(color.a, color.r, color.g, color.b));
	graphics.DrawLine(&pen, (int)x1, (int)y1, (int)x2, (int)y2);
}

void Image::DrawString(const std::wstring& text, ColorType color, unsigned int x, unsigned int y, Font& font)
{
	Gdiplus::Graphics graphics(bitmap.get());
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
	Gdiplus::SolidBrush brush(Gdiplus::Color(color.a, color.r, color.g, color.b));

	Gdiplus::PointF pnt(x, y);
	graphics.DrawString(text.c_str(), text.size(), font.font.get(), pnt, &brush);
}

void Image::DrawImage(const Image& img , unsigned int x , unsigned int y)
{
	Gdiplus::Graphics graphics(bitmap.get());
	graphics.DrawImage(img.bitmap.get(), (INT)x, (INT)y);
}

void Image::Clear(ColorType color)
{
	Gdiplus::Graphics graphics(bitmap.get());
	graphics.Clear(Gdiplus::Color(color.a,color.r,color.g,color.b));
}

ColorType* Image::Raw()
{
	Gdiplus::BitmapData data;
	auto bitmapPtr = bitmap.get();
	Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
	bitmapPtr->LockBits(&rect, Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB , &data);
	bitmapPtr->UnlockBits(&data);
	return static_cast<ColorType*>(data.Scan0);
}

const ColorType* Image::Raw() const
{
	Gdiplus::BitmapData data;
	auto bitmapPtr = bitmap.get();
	Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
	bitmapPtr->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data);
	bitmapPtr->UnlockBits(&data);
	return static_cast<ColorType*>(data.Scan0);
}

Image& Image::operator=(const Image& img)
{
	bitmap = std::make_unique<Gdiplus::Bitmap>(img.GetWidth(), img.GetHeight(), PixelFormat32bppARGB);
	Gdiplus::Graphics graphics(bitmap.get());
	graphics.DrawImage(img.bitmap.get(), 0, 0, img.GetWidth(), img.GetHeight());
	return *this;
}

void Image::get_decoder_from_ext(const std::string& ext, CLSID& Clsid)
{
	HRESULT hResult;

	if (ext == ".png")
	{
		hResult = CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &Clsid);
	}
	else if (ext == ".jpg" || ext == ".jpeg")
	{
		hResult = CLSIDFromString(L"{557CF401-1A04-11D3-9A73-0000F81EF32E}", &Clsid);
	}
	else if (ext == ".bmp")
	{
		hResult = CLSIDFromString(L"{557CF400-1A04-11D3-9A73-0000F81EF32E}", &Clsid);
	}
	else if (ext == ".tif")
	{
		hResult = CLSIDFromString(L"{557CF405-1A04-11D3-9A73-0000F81EF32E}", &Clsid);
	}
	else
	{
		throw std::runtime_error("Invalid file extension");
	}

	if (FAILED(hResult))
	{
		throw std::runtime_error("Failed to get Encoder");
	}
}

std::vector<char> Image::SaveToBuffer(const std::string ext_type) const
{
	std::vector<char> buffer;
	CLSID Clsid;
	get_decoder_from_ext(ext_type, Clsid);
	Microsoft::WRL::ComPtr<IStream> istream;

	if (CreateStreamOnHGlobal(NULL, TRUE, &istream) != S_OK)
	{

	}
	if (bitmap->Save(istream.Get(), &Clsid) != Gdiplus::Status::Ok)
	{
		throw std::runtime_error("Failed to save to stream");
	}

	HGLOBAL hg = NULL;
	if (GetHGlobalFromStream(istream.Get(), &hg) != S_OK)
	{
		throw std::runtime_error("Failed to get global from stream");
	}

	int bufsize = GlobalSize(hg);
	buffer.resize(bufsize);

	LPVOID ptr = GlobalLock(hg);
	if(ptr != nullptr)
		memcpy(buffer.data(), ptr, bufsize);
	GlobalUnlock(hg);
	return buffer;
}

void Image::Save(const std::filesystem::path& file) const
{
	CLSID Clsid;
	get_decoder_from_ext(file.extension().string(), Clsid);
	auto status = bitmap->Save(file.wstring().c_str(),&Clsid);

	if (status != Gdiplus::Status::Ok)
	{
		throw std::runtime_error("Failed to save image");
	}
}
