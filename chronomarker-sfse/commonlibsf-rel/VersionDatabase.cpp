#include "VersionDatabase.h"
#include <windows.h>
#include <filesystem>
#include <regex>
#include <format>
#pragma comment(lib,"Version.lib") 

extern "C" IMAGE_DOS_HEADER __ImageBase;

void *GetCurrentModule() noexcept
{
	return std::addressof(__ImageBase);
}

[[nodiscard]] inline auto utf8_to_utf16(const std::string_view a_in) noexcept -> std::optional<std::wstring>
{
	const auto cvt = [&](wchar_t *a_dst, const std::size_t a_length) {
		return MultiByteToWideChar(
			CP_UTF8, 0, a_in.data(), static_cast<int>(a_in.length()), a_dst, static_cast<int>(a_length));
		};

	const auto len = cvt(nullptr, 0);
	if (len == 0) {
		return std::nullopt;
	}

	std::wstring out(len, '\0');
	if (cvt(out.data(), out.length()) == 0) {
		return std::nullopt;
	}

	return out;
}

[[nodiscard]] inline auto utf16_to_utf8(const std::wstring_view a_in) noexcept -> std::optional<std::string>
{
	const auto cvt = [&](char *a_dst, const std::size_t a_length) {
		return WideCharToMultiByte(
			CP_UTF8, 0, a_in.data(), static_cast<int>(a_in.length()), a_dst, static_cast<int>(a_length), nullptr, nullptr);
		};

	const auto len = cvt(nullptr, 0);
	if (len == 0) {
		return std::nullopt;
	}

	std::string out(len, '\0');
	if (cvt(out.data(), out.length()) == 0) {
		return std::nullopt;
	}

	return out;
}

inline bool report_and_error(std::string_view a_msg, const bool a_fail = true, const std::source_location &a_loc = std::source_location::current())
{
	const auto body = [&]() -> std::wstring {
		const std::filesystem::path p = a_loc.file_name();
		auto                        filename = p.lexically_normal().generic_string();

		const std::regex r{ R"((?:^|[\\\/])(?:include|src)[\\\/](.*)$)" };
		std::smatch      matches;
		if (std::regex_search(filename, matches, r)) {
			filename = matches[1].str();
		}

		return utf8_to_utf16(std::format("{}({}): {}"sv, filename, a_loc.line(), a_msg)).value_or(L"<character encoding error>"s);
		}();

	const auto caption = []() {
		const auto           maxPath = MAX_PATH;
		std::vector<wchar_t> buf;
		buf.reserve(maxPath);
		buf.resize(maxPath / 2);
		std::uint32_t result;
		do {
			buf.resize(buf.size() * 2);
			result = GetModuleFileNameW((HMODULE)GetCurrentModule(), buf.data(), static_cast<std::uint32_t>(buf.size()));
		} while (result && result == buf.size() && buf.size() <= (std::numeric_limits<std::uint32_t>::max)());

		if (result && result != buf.size()) {
			const std::filesystem::path p(buf.begin(), buf.begin() + result);
			return p.filename().native();
		}
		return L""s;
		}();

	if (a_fail) {
		MessageBoxW(nullptr, body.c_str(), (caption.empty() ? nullptr : caption.c_str()), 0);
		TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
	}
	return true;
}

[[noreturn]] void report_and_fail(const std::string_view a_msg, const std::source_location &a_loc)
{
	report_and_error(a_msg, true, a_loc);
	std::unreachable();
}

[[nodiscard]] inline std::optional<REL::Version> get_file_version(const std::wstring a_filename)
{
	unsigned long     dummy{};
	std::vector<char> buf(GetFileVersionInfoSizeW(a_filename.data(), std::addressof(dummy)));
	if (buf.empty()) {
		return std::nullopt;
	}

	if (!GetFileVersionInfoW(a_filename.data(), 0, buf.size(), buf.data())) {
		return std::nullopt;
	}

	void *verBuf{};
	std::uint32_t verLen{};
	if (!VerQueryValueW(buf.data(), L"\\StringFileInfo\\040904B0\\ProductVersion", std::addressof(verBuf), std::addressof(verLen))) {
		return std::nullopt;
	}

	REL::Version             version;
	std::wistringstream ss(std::wstring(static_cast<const wchar_t *>(verBuf), verLen));
	std::wstring        token;
	for (std::size_t i = 0; i < 4 && std::getline(ss, token, L'.'); ++i) {
		version[i] = static_cast<std::uint16_t>(std::stoi(token));
	}

	return version;
}

constexpr auto LookUpDir = "Data\\SFSE\\Plugins"sv;

namespace REL {

	std::uintptr_t ID::base()
	{
		return (std::uintptr_t)GetModuleHandleA(nullptr);
	}


	bool memory_map::open(const std::wstring a_name, const std::size_t a_size)
	{
		close();

		_mapping = OpenFileMappingW(
			FILE_MAP_READ | FILE_MAP_WRITE,
			false,
			a_name.data());

		if (!_mapping) {
			close();
			return false;
		}

		_view = MapViewOfFile(
			_mapping,
			FILE_MAP_READ | FILE_MAP_WRITE,
			0,
			0,
			a_size);

		if (!_view) {
			close();
			return false;
		}

		return true;
	}

	bool memory_map::create(const std::wstring a_name, const std::size_t a_size)
	{
		close();

		ULARGE_INTEGER bytes;
		bytes.QuadPart = a_size;

		_mapping = OpenFileMappingW(
			FILE_MAP_READ | FILE_MAP_WRITE,
			false,
			a_name.data());

		if (!_mapping) {
			_mapping = CreateFileMappingW(
				INVALID_HANDLE_VALUE,
				nullptr,
				PAGE_READWRITE,
				bytes.HighPart,
				bytes.LowPart,
				a_name.data());

			if (!_mapping) {
				return false;
			}
		}

		_view = MapViewOfFile(
			_mapping,
			FILE_MAP_READ | FILE_MAP_WRITE,
			0,
			0,
			bytes.QuadPart);

		if (!_view) {
			return false;
		}

		return true;
	}

	constexpr void memory_map::close()
	{
		if (_view) {
			(void)UnmapViewOfFile(_view);
			_view = nullptr;
		}

		if (_mapping) {
			(void)CloseHandle(_mapping);
			_mapping = nullptr;
		}
	}

	void header_t::read(istream_t &a_in)
	{
		std::uint32_t format{};
		a_in.readin(format);

		stl_assert(format == kDatabaseVersion,
			"Unsupported address library format: {}\n"
			"Compiled IDDatabase version: {}\n"
			"This means this script extender plugin is incompatible with the address "
			"library available for this version of the game, and thus does not support it."sv,
			format, std::to_underlying(kDatabaseVersion));

		std::uint32_t version[4]{};
		std::uint32_t nameLen{};
		a_in.readin(version);
		a_in.readin(nameLen);

		for (std::uint32_t i = 0; i < nameLen; ++i) {
			a_in.readin(_name[i]);
		}
		_name[nameLen] = '\0';

		a_in.readin(_pointerSize);
		a_in.readin(_addressCount);

		for (std::size_t i = 0; i < std::extent_v<decltype(version)>; ++i) {
			_version[i] = static_cast<std::uint16_t>(version[i]);
		}
	}

[[nodiscard]] std::size_t IDDatabase::id2offset(std::uint64_t a_id) const
{
	const mapping_t elem{ a_id, 0 };
	const auto                it = std::ranges::lower_bound(
		_id2offset,
		elem,
		[](auto &&a_lhs, auto &&a_rhs) {
			return a_lhs.id < a_rhs.id;
		});

	stl_assert(it != _id2offset.end(),
		"Failed to find the id within the address library: {}\n"
		"Compiled IDDatabase version: {}\n"
		"This means this script extender plugin is incompatible with the address "
		"library for this version of the game, and thus does not support it."sv,
		a_id, std::to_underlying(kDatabaseVersion));

	return it->offset;
}

std::string_view GetProcPath(
	HMODULE a_handle) noexcept
{
	static std::string fileName(MAX_PATH + 1, ' ');
	auto               res = GetModuleFileName(a_handle, fileName.data(), MAX_PATH + 1);
	if (res == 0) {
		fileName = "[ProcessHost]";
		res = 13;
	}

	return { fileName.c_str(), res };
}


std::wstring IDDatabase::addresslib_filename(const REL::Version &version)
{
	// sfse only loads plugins from { runtimeDirectory + "Data\\SFSE\\Plugins" }
	auto file = std::filesystem::current_path();

	file /= std::format("{}\\versionlib-{}", LookUpDir, version.string("-"));

	_platform = Platform::kUnknown;
	if (GetModuleHandle("steam_api64")) {
		_platform = Platform::kSteam;
		_is_steam = true;
	}
	else {
		_platform = Platform::kMsStore;
	}

	stl_assert(_platform != Platform::kUnknown,
		"Failed to identify game runtime platform,"
		"This means the address library is incompatible for this platform."sv);

	// steam version omits the suffix
	if (_platform != Platform::kSteam) {
		file += std::format("-{}", std::to_underlying(_platform));
	}
	file += ".bin";

	stl_assert(std::filesystem::exists(file),
		"Failed to find address library file: \n{}", file.string());

	return file.wstring();
}

void IDDatabase::load()
{
	auto _file = utf8_to_utf16(GetProcPath(nullptr)).value();
	const auto version = get_file_version(_file).value();
	const auto file = addresslib_filename(version);
	load_file(file, version, true);
}

bool IDDatabase::load_file(const std::wstring a_filename, const Version a_version, const bool a_failOnError)
{
	try {
		istream_t in(a_filename.data(), std::ios::in | std::ios::binary);
		header_t  header;
		header.read(in);

		if (header.version() != a_version) {
			report_and_error(
				std::format(
					"Address library version mismatch.\n"
					"Read-in: {}"sv,
					header.version().string()),
				a_failOnError);
		}

		const auto mapname = utf8_to_utf16(std::format(
			// kDatabaseVersion, runtimeVersion, runtimePlatform
			"CommonLibSF-Offsets-v{}-{}-{}",
			std::to_underlying(kDatabaseVersion),
			a_version.string("-"),
			std::to_underlying(_platform)));

		stl_assert(mapname.has_value(),
			"Failed to generate memory map name.");

		const auto byteSize = header.address_count() * sizeof(mapping_t);
		if (_mmap.open(mapname.value(), byteSize)) {
			_id2offset = { static_cast<mapping_t *>(_mmap.data()), header.address_count() };
		}
		else if (_mmap.create(mapname.value(), byteSize)) {
			_id2offset = { static_cast<mapping_t *>(_mmap.data()), header.address_count() };
			unpack_file(in, header, a_failOnError);
			std::ranges::sort(
				_id2offset,
				[](auto &&a_lhs, auto &&a_rhs) {
					return a_lhs.id < a_rhs.id;
				});
		}
		else {
			return report_and_error("failed to create shared mapping"sv, a_failOnError);
		}
	}
	catch (const std::system_error &) {
		return report_and_error(
			std::format(
				"Failed to locate an appropriate address library with the path: {}\n"
				"This means you are either missing the address library for this "
				"specific version of the game or running on unsupported platform.\n"
				"Please continue to the mod page for address library to download "
				"an appropriate version or platform. \nIf one is not available, "
				"then it is likely that address library has not yet added support "
				"for this version of the game or this platform.\n"
				"Current version: {}\n"
				"Current platform: {}"sv,
				utf16_to_utf8(a_filename).value_or("<unknown filename>"s),
				a_version.string(),
				_is_steam ? "Steam" : "Microsoft Store"),
			a_failOnError);
	}
	return true;
}

bool IDDatabase::unpack_file(istream_t &a_in, const header_t &a_header, const bool a_failOnError)
{
	std::uint8_t  type = 0;
	std::uint64_t id = 0;
	std::uint64_t offset = 0;
	std::uint64_t prevID = 0;
	std::uint64_t prevOffset = 0;
	for (auto &mapping : _id2offset) {
		a_in.readin(type);
		const auto lo = static_cast<std::uint8_t>(type & 0xF);
		const auto hi = static_cast<std::uint8_t>(type >> 4);

		switch (lo) {
		case 0:
			a_in.readin(id);
			break;
		case 1:
			id = prevID + 1;
			break;
		case 2:
			id = prevID + a_in.readout<std::uint8_t>();
			break;
		case 3:
			id = prevID - a_in.readout<std::uint8_t>();
			break;
		case 4:
			id = prevID + a_in.readout<std::uint16_t>();
			break;
		case 5:
			id = prevID - a_in.readout<std::uint16_t>();
			break;
		case 6:
			id = a_in.readout<std::uint16_t>();
			break;
		case 7:
			id = a_in.readout<std::uint32_t>();
			break;
		default:
			return report_and_error("unhandled type"sv, a_failOnError);
		}

		const std::uint64_t tmp = (hi & 8) != 0 ? (prevOffset / a_header.pointer_size()) : prevOffset;

		switch (hi & 7) {
		case 0:
			a_in.readin(offset);
			break;
		case 1:
			offset = tmp + 1;
			break;
		case 2:
			offset = tmp + a_in.readout<std::uint8_t>();
			break;
		case 3:
			offset = tmp - a_in.readout<std::uint8_t>();
			break;
		case 4:
			offset = tmp + a_in.readout<std::uint16_t>();
			break;
		case 5:
			offset = tmp - a_in.readout<std::uint16_t>();
			break;
		case 6:
			offset = a_in.readout<std::uint16_t>();
			break;
		case 7:
			offset = a_in.readout<std::uint32_t>();
			break;
		default:
			return report_and_error("unhandled type"sv, a_failOnError);
		}

		if ((hi & 8) != 0) {
			offset *= a_header.pointer_size();
		}

		mapping = { id, offset };

		prevOffset = offset;
		prevID = id;
	}
	return true;
}

IDDatabase IDDatabase::_instance;
}
