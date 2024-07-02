#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <fstream>
#include <span>
#include <string>
#include <string_view>
#include <source_location>


using namespace std::string_literals;
using namespace std::string_view_literals;


#define AsAddress(ptr) std::bit_cast<std::uintptr_t>(ptr)
#define AsPointer(addr) std::bit_cast<void*>(addr)
#define stl_assert(cond, ...)                                     \
	{                                                             \
		if (!((cond))) {                                          \
			report_and_fail(std::format(__VA_ARGS__)); \
		}                                                         \
	}

void report_and_fail(const std::string_view a_msg, const std::source_location &a_loc = std::source_location::current());

namespace REL
{
	class Version
	{
	public:
		using value_type = std::uint16_t;
		using reference = value_type &;
		using const_reference = const value_type &;

		constexpr Version() noexcept = default;

		explicit constexpr Version(const std::array<value_type, 4> a_version) noexcept :
			_impl(a_version)
		{}

		constexpr Version(const value_type a_v1, const value_type a_v2 = 0, const value_type a_v3 = 0, const value_type a_v4 = 0) noexcept :
			_impl{ a_v1, a_v2, a_v3, a_v4 }
		{}

		explicit constexpr Version(const std::string_view a_version)
		{
			std::array<value_type, 4> powers{ 1, 1, 1, 1 };
			std::size_t               position{};
			for (const auto &c : a_version) {
				if (c == '.') {
					if (++position == powers.size()) {
						throw std::invalid_argument("Too many parts in version number.");
					}
				}
				else {
					powers[position] *= 10;
				}
			}
			position = 0;
			for (const auto &c : a_version) {
				if (c == '.') {
					++position;
				}
				else if (c < '0' || c > '9') {
					throw std::invalid_argument("Invalid character in version number.");
				}
				else {
					powers[position] /= 10;
					_impl[position] += static_cast<value_type>((c - '0') * powers[position]);
				}
			}
		}

		[[nodiscard]] constexpr reference operator[](const std::size_t a_idx) noexcept { return _impl[a_idx]; }

		[[nodiscard]] constexpr const_reference operator[](const std::size_t a_idx) const noexcept { return _impl[a_idx]; }

		[[nodiscard]] constexpr decltype(auto) begin() const noexcept { return _impl.begin(); }

		[[nodiscard]] constexpr decltype(auto) cbegin() const noexcept { return _impl.cbegin(); }

		[[nodiscard]] constexpr decltype(auto) end() const noexcept { return _impl.end(); }

		[[nodiscard]] constexpr decltype(auto) cend() const noexcept { return _impl.cend(); }

		[[nodiscard]] constexpr std::strong_ordering compare(const Version &a_rhs) const noexcept
		{
			for (std::size_t i = 0; i < _impl.size(); ++i) {
				if ((*this)[i] != a_rhs[i]) {
					return (*this)[i] < a_rhs[i] ? std::strong_ordering::less : std::strong_ordering::greater;
				}
			}
			return std::strong_ordering::equal;
		}

		[[nodiscard]] constexpr std::uint32_t pack() const noexcept
		{
			return static_cast<std::uint32_t>(
				(_impl[0] & 0x0FF) << 24u |
				(_impl[1] & 0x0FF) << 16u |
				(_impl[2] & 0xFFF) << 4u |
				(_impl[3] & 0x00F) << 0u);
		}

		[[nodiscard]] constexpr value_type major() const noexcept { return _impl[0]; }

		[[nodiscard]] constexpr value_type minor() const noexcept { return _impl[1]; }

		[[nodiscard]] constexpr value_type patch() const noexcept { return _impl[2]; }

		[[nodiscard]] constexpr value_type build() const noexcept { return _impl[3]; }

		[[nodiscard]] constexpr std::string string(const std::string_view a_separator = "."sv) const
		{
			std::string result;
			for (auto &&ver : _impl) {
				result += std::to_string(ver);
				result.append(a_separator.data(), a_separator.size());
			}
			result.erase(result.size() - a_separator.size(), a_separator.size());
			return result;
		}

		[[nodiscard]] constexpr std::wstring wstring(const std::wstring_view a_separator = L"."sv) const
		{
			std::wstring result;
			for (auto &&ver : _impl) {
				result += std::to_wstring(ver);
				result.append(a_separator.data(), a_separator.size());
			}
			result.erase(result.size() - a_separator.size(), a_separator.size());
			return result;
		}

		[[nodiscard]] static constexpr Version unpack(const std::uint32_t a_packedVersion) noexcept
		{
			return Version{
				static_cast<value_type>((a_packedVersion >> 24) & 0x0FF),
				static_cast<value_type>((a_packedVersion >> 16) & 0x0FF),
				static_cast<value_type>((a_packedVersion >> 4) & 0xFFF),
				static_cast<value_type>(a_packedVersion & 0x0F)
			};
		}

	private:
		std::array<value_type, 4> _impl{ 0, 0, 0, 0 };
	};

	[[nodiscard]] constexpr bool operator==(const Version &a_lhs, const Version &a_rhs) noexcept
	{
		return a_lhs.compare(a_rhs) == std::strong_ordering::equal;
	}

	[[nodiscard]] constexpr std::strong_ordering operator<=>(const Version &a_lhs, const Version &a_rhs) noexcept
	{
		return a_lhs.compare(a_rhs);
	}

	class istream_t
	{
	public:
		using stream_type = std::ifstream;
		using pointer = stream_type *;
		using const_pointer = const stream_type *;
		using reference = stream_type &;
		using const_reference = const stream_type &;

		istream_t(const std::wstring a_filename, const std::ios_base::openmode a_mode) :
			_stream(a_filename.data(), a_mode)
		{
			stl_assert(_stream.is_open(),
				"failed to open address library file");

			_stream.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
		}

		void ignore(const std::streamsize a_count)
		{
			_stream.ignore(a_count);
		}

		template <class T>
		void readin(T &a_val)
		{
			_stream.read(std::bit_cast<char *>(std::addressof(a_val)), sizeof(T));
		}

		template <class T>
			requires(std::is_arithmetic_v<T>)
		T readout()
		{
			T val{};
			readin(val);
			return val;
		}

	private:
		stream_type _stream;
	};

	class header_t
	{
	public:
		void read(istream_t &a_in);

		[[nodiscard]] constexpr std::size_t address_count() const noexcept
		{
			return _addressCount;
		}

		[[nodiscard]] constexpr std::uint64_t pointer_size() const noexcept
		{
			return _pointerSize;
		}

		[[nodiscard]] constexpr std::string_view name() const noexcept
		{
			return _name;
		}

		[[nodiscard]] constexpr Version version() const noexcept
		{
			return _version;
		}

	private:
		char          _name[20]{};
		Version       _version;
		std::uint32_t _pointerSize{};
		std::uint32_t _addressCount{};
	};

	enum : std::uint32_t
	{
		kDatabaseVersion = 2
	};

	struct mapping_t
	{
		std::uint64_t id;
		std::uint64_t offset;
	};

	class memory_map
	{
	public:
		constexpr memory_map() noexcept = default;

		constexpr memory_map(const memory_map &) = delete;

		constexpr memory_map(memory_map &&a_rhs) noexcept :
			_mapping(a_rhs._mapping),
			_view(a_rhs._view)
		{
			a_rhs._mapping = nullptr;
			a_rhs._view = nullptr;
		}

		~memory_map() { close(); }

		constexpr memory_map &operator=(const memory_map &) = delete;

		constexpr memory_map &operator=(memory_map &&a_rhs) noexcept
		{
			if (this != std::addressof(a_rhs)) {
				_mapping = a_rhs._mapping;
				a_rhs._mapping = nullptr;

				_view = a_rhs._view;
				a_rhs._view = nullptr;
			}
			return *this;
		}

		[[nodiscard]] constexpr void *data() const noexcept { return _view; }

		bool open(std::wstring a_name, std::size_t a_size);

		bool create(std::wstring a_name, std::size_t a_size);

		constexpr void close();

	private:
		void *_mapping{};
		void *_view{};
	};

	class IDDatabase
	{
	public:
		enum class Platform
		{
			kUnknown = -1,
			kSteam,
			kMsStore,
		};

		static IDDatabase &get()
		{
			if (_initialized.load(std::memory_order_relaxed)) {
				return _instance;
			}
			[[maybe_unused]] std::unique_lock lock(_initLock);
			_instance.load();
			_initialized.store(true, std::memory_order_relaxed);
			return _instance;
		}

		[[nodiscard]] std::size_t id2offset(std::uint64_t a_id) const;

	private:
		friend class Module;

		constexpr IDDatabase() = default;

		~IDDatabase() = default;

		constexpr IDDatabase(const IDDatabase &) = delete;

		constexpr IDDatabase(IDDatabase &&) = delete;

		constexpr IDDatabase &operator=(const IDDatabase &) = delete;

		constexpr IDDatabase &operator=(IDDatabase &&) = delete;

		std::wstring addresslib_filename(const REL::Version &version);

		void load();

		bool load_file(std::wstring a_filename, Version a_version, bool a_failOnError);

		bool unpack_file(istream_t &a_in, const header_t &a_header, bool a_failOnError);

		void clear()
		{
			_mmap.close();
			_id2offset = {};
		}

		static IDDatabase              _instance;
		inline static std::atomic_bool _initialized{};
		inline static std::mutex       _initLock;
		inline static bool             _is_steam{};
		memory_map           _mmap;
		std::span<mapping_t> _id2offset;
		Platform                       _platform{ Platform::kUnknown };
	};

	class ID
	{
	public:
		constexpr ID() noexcept = default;

		explicit constexpr ID(const std::uint64_t a_id) noexcept :
			_id(a_id)
		{}

		constexpr ID &operator=(const std::uint64_t a_id) noexcept
		{
			_id = a_id;
			return *this;
		}

		[[nodiscard]] std::uintptr_t address() const
		{
			return base() + offset();
		}

		[[nodiscard]] constexpr std::uint64_t id() const noexcept
		{
			return _id;
		}

		[[nodiscard]] std::size_t offset() const
		{
			return IDDatabase::get().id2offset(_id);
		}

	private:
		[[nodiscard]] static std::uintptr_t base();

		std::uint64_t _id{};
	};
}
