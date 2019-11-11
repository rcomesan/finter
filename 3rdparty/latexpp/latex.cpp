#include "latex.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <libplatform/libplatform.h>
#include <v8.h>
#include <wkhtmltox/image.h>
#include <windows.h>

Latex::V8 Latex::_v8;

Latex::Latex(WarningBehavior behavior)
: Latex("katex/katex.min.css", behavior)
{ }

Latex::Latex(const std::string& stylesheet, WarningBehavior behavior)
: _stylesheet(stylesheet)
, _warning_behaviour(behavior)
, _isolate(_new_isolate())
{
	v8::HandleScope handle_scope(_isolate);
	
	v8::Isolate::Scope isolate_scope(_isolate);
	
	auto context = v8::Context::New(_isolate);
	
	v8::Context::Scope context_scope(context);
	
	_load_katex(context);
	
	_persistent_context = v8::UniquePersistent<v8::Context>(_isolate, context);
	
	wkhtmltoimage_init(false);
}

Latex::Latex(const Latex& other)
: Latex(other._stylesheet,
		other._warning_behaviour)
{
	_additional_css = other._additional_css;
}

Latex::Latex(Latex&& other) noexcept
: Latex()
{
	swap(other);
}

Latex& Latex::operator=(Latex other)
{
	swap(other);
	
	return *this;
}

std::string & Latex::exe_folder_path()
{
    static std::string _p;

    if (_p.empty())
    {
        HMODULE hModule = GetModuleHandleW(NULL);
        char path[MAX_PATH];
        GetModuleFileName(hModule, path, MAX_PATH);
        (*strrchr(path, '\\')) = '\0';
        _p = std::string(path) + "\\";
    }

    return _p;
}

std::string& Latex::tmp_html_path()
{
    static std::string _p;
    if (_p.empty())
    {
        _p = Latex::exe_folder_path() + "tmp.html";
    }

    return _p;
}

std::string& Latex::tmp_png_path()
{
    static std::string _p;
    if (_p.empty())
    {
        _p = Latex::exe_folder_path() + "tmp.png";
    }

    return _p;
}


void Latex::swap(Latex &other) noexcept
{
	// Enable ADL
	using std::swap;
	
	swap(_allocator, other._allocator);
	
	swap(_isolate, other._isolate);
	
	swap(_persistent_context, other._persistent_context);
	
	swap(_stylesheet, other._stylesheet);
	
	swap(_additional_css, other._additional_css);
	
	swap(_warning_behaviour, other._warning_behaviour);
}

void swap(Latex& first, Latex& second) noexcept
{
	first.swap(second);
}

Latex::~Latex()
{
	wkhtmltoimage_deinit();
}

std::string Latex::to_html(const std::string& latex) const
{
	static const std::string arguments = "{'displayMode': true}";
	
	v8::Isolate::Scope isolate_scope(_isolate);
	
	// Stack-allocated handle-scope (takes care of handles such
	// that object are garbage-collected after the scope ends)
	v8::HandleScope handle_scope(_isolate);
	
	// Get a local context handle from the persistent handle.
	auto context = v8::Local<v8::Context>::New(_isolate,
											   _persistent_context);
	
	v8::Context::Scope context_scope(context);
	
	std::string source = "katex.renderToString('";
	
	source += _escape(latex) + "', ";
	source += arguments + ");";
	
	auto value = _run(source, context);
	
    std::string html = *v8::String::Utf8Value(v8::Isolate::GetCurrent(), value);
	
	return "<div class='latex'>\n" + html + "</div>\n";
}

std::string Latex::to_complete_html(const std::string &latex) const
{
	auto snippet = to_html(latex);
	
	std::string html = "<!DOCTYPE html>\n<html>\n";
	
	html += "<head>\n<meta charset='utf-8'/>\n";
	html += "<link rel='stylesheet' type='text/css' ";
 	html += "href='" + _stylesheet + "'>\n";
	
	if (! _additional_css.empty())
	{
		html += "<style>\n";
		html += _additional_css;
		html += "</style>\n";
	}
	
	html += "</head>\n<body>\n";
	html += snippet;
	html += "</body>\n</html>";
	
	return html;
}

void Latex::to_image(const std::string &latex,
				  const std::string &filepath,
				  ImageFormat format) const
{
    std::ofstream temp(Latex::tmp_html_path());
	
	if (! temp) throw FileException("Could not open temporary file!");
	
	temp << to_complete_html(latex);

	temp.close();
	
	auto converter = _new_converter(filepath, format);
	
	if (! wkhtmltoimage_convert(converter))
	{
		throw ConversionException("Could not convert to png!");
	}
	
	wkhtmltoimage_destroy_converter(converter);
	
	//remove(Latex::tmp_html_path().c_str());
}

void Latex::to_png(const std::string &latex,
				const std::string &filepath) const
{
	to_image(latex, filepath, ImageFormat::PNG);
}

void Latex::to_jpg(const std::string &latex,
				const std::string &filepath) const
{
	to_image(latex, filepath, ImageFormat::JPG);
}

void Latex::to_svg(const std::string &latex,
				const std::string &filepath) const
{
	to_image(latex, filepath, ImageFormat::SVG);
}

void Latex::add_css(const std::string& css)
{
	_additional_css += css;
}

const std::string& Latex::additional_css() const
{
	return _additional_css;
}

void Latex::clear_additional_css()
{
	_additional_css.clear();
}

const std::string& Latex::stylesheet() const
{
	return _stylesheet;
}

void Latex::stylesheet(const std::string& stylesheet)
{
	_stylesheet = stylesheet;
}

const Latex::WarningBehavior& Latex::warning_behavior() const
{
	return _warning_behaviour;
}

void Latex::warning_behavior(WarningBehavior behavior)
{
	_warning_behaviour = behavior;
}

v8::Isolate* Latex::_new_isolate() const
{
	v8::Isolate::CreateParams parameters;
	
	parameters.array_buffer_allocator = &_allocator;
	
	// Isolated JavaScript Virtual Environment
	return v8::Isolate::New(parameters);
}

void Latex::_load_katex(const v8::Local<v8::Context>& context) const
{
    std::ifstream file(Latex::exe_folder_path() + "katex\\katex.min.js");
	
	std::string source;
	std::string input;
	
	while (std::getline(file, input))
	{
		source += input + " ";
	}
	
	_run(source, context);
}

v8::Local<v8::Value> Latex::_run(const std::string& source,
								 const v8::Local<v8::Context>& context) const
{
	v8::EscapableHandleScope handle_scope(_isolate);
	
	auto unchecked = v8::String::NewFromUtf8(_isolate,
											 source.c_str(),
											 v8::NewStringType::kNormal);
	
	auto checked = unchecked.ToLocalChecked();

	// Compile the source code.
	auto script = v8::Script::Compile(context, checked).ToLocalChecked();
	
	// V8 engine's try-catch mechanism
	v8::TryCatch try_catch(_isolate);
	
	auto result = script->Run(context);
	
	if (result.IsEmpty())
	{
		// Grab last exception
		auto exception = try_catch.Exception();
		
		std::string what = *v8::String::Utf8Value(v8::Isolate::GetCurrent(), exception);
		
		// Remove the 'ParseError' (redundant)
		throw ParseException(what.substr(12));
	}
	
	// Allows us to return local-scope objects to the outside scope
	return handle_scope.Escape(result.ToLocalChecked());
}

std::string Latex::_escape(std::string source) const
{
	for (auto i = source.begin(); i != source.end(); ++i)
	{
		if (*i == '\\')
		{
			i = source.insert(i, '\\');
			
			++i;
		}
	}
	
	return source;
}

void _throw(wkhtmltoimage_converter*, const char* message)
{
	throw Latex::ConversionException(message);
}


void _log(wkhtmltoimage_converter*, const char* message)
{
	std::clog << message << std::endl;
}

wkhtmltoimage_converter*
Latex::_new_converter(const std::string& filepath, ImageFormat format) const
{
	auto settings = _new_converter_settings(filepath, format);
	
	auto converter = wkhtmltoimage_create_converter(settings, nullptr);
	
	wkhtmltoimage_set_error_callback(converter, _throw);
	
	if (_warning_behaviour == WarningBehavior::Strict)
	{
		wkhtmltoimage_set_error_callback(converter, _throw);
	}
	
	else if (_warning_behaviour == WarningBehavior::Log)
	{
		wkhtmltoimage_set_warning_callback(converter, _log);
	}
	
	return converter;
}

wkhtmltoimage_global_settings*
Latex::_new_converter_settings(const std::string& filepath,
					 Latex::ImageFormat format) const
{
	auto settings = wkhtmltoimage_create_global_settings();
	
	wkhtmltoimage_set_global_setting(settings,
									 "transparent",
									 "false");
	
	wkhtmltoimage_set_global_setting(settings,
									 "in",
                                        Latex::tmp_html_path().c_str());
	
	wkhtmltoimage_set_global_setting(settings,
									 "out",
									 filepath.c_str());
	const char* fmt{ "png" };
	
	switch (format)
	{
		case ImageFormat::PNG: fmt = "png"; break;
			
		case ImageFormat::JPG: fmt = "jpg"; break;
			
		case ImageFormat::SVG: fmt = "svg"; break;
	}
	
	wkhtmltoimage_set_global_setting(settings,
									 "fmt",
									 fmt);
	
	wkhtmltoimage_set_global_setting(settings,
									 "screenWidth",
									 "0");
	
	wkhtmltoimage_set_global_setting(settings,
									 "quality",
									 "100");
	
	return settings;
}

Latex::V8::V8()
{    
	v8::V8::InitializeICUDefaultLocation(Latex::exe_folder_path().c_str());
    v8::V8::InitializeExternalStartupData(Latex::exe_folder_path().c_str());
    platform = v8::platform::NewDefaultPlatform();
	v8::V8::InitializePlatform(platform.get());
	v8::V8::Initialize();
}

Latex::V8::~V8()
{
	v8::V8::Dispose();
	v8::V8::ShutdownPlatform();
}

void* Latex::Allocator::Allocate(size_t length)
{
	auto data = AllocateUninitialized(length);
	
	return data ? std::memset(data, 0, length) : data;
}

void Latex::Allocator::Free(void *data, size_t)
{
	free(data);
}

void* Latex::Allocator::AllocateUninitialized(size_t length)
{
	return malloc(length);
}