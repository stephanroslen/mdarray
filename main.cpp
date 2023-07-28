#include <array>
#include <cstddef>
#include <format>
#include <iostream>
#include <numeric>

namespace MDArrayDetail {

template<size_t dimValue>
struct DimIndexResult {
  static constexpr size_t dim = dimValue;
  size_t index;
};

template<size_t... dims>
struct DimHelper;

template<size_t dimFirst>
struct DimHelper<dimFirst> {
  static constexpr auto toIdx(const size_t index) noexcept { return DimIndexResult<dimFirst>{index}; }
};

template<size_t dimFirst, size_t... dimResidual>
struct DimHelper<dimFirst, dimResidual...> {
  template<typename... IndexResidual>
  static constexpr auto toIdx(const size_t index, const IndexResidual... indexResidual) noexcept
    requires((sizeof...(dimResidual) == sizeof...(IndexResidual)) && (std::is_same_v<size_t, IndexResidual> && ...))
  {
    using FwdDimHelper = DimHelper<dimResidual...>;
    const auto fwd{FwdDimHelper::toIdx(indexResidual...)};

    using DIR = decltype(fwd);

    return DimIndexResult<dimFirst * DIR::dim>{index + dimFirst * fwd.index};
  }
};

} // namespace MDArrayDetail

template<typename T, size_t... dims>
struct MDArray : std::array<T, (dims * ...)> {
  using Base = std::array<T, (dims * ...)>;
  using Base::at;
  using Base::Base;
  using Base::operator[];

  template<typename... Indices>
  auto& at(const Indices... indices) noexcept
    requires((sizeof...(dims) > 1) && (sizeof...(dims) == sizeof...(Indices))
             && (std::is_same_v<size_t, Indices> && ...))
  {
    return Base::at(toIdx(indices...));
  }

  template<typename... Indices>
  const auto& at(const Indices... indices) const noexcept
    requires((sizeof...(dims) > 1) && (sizeof...(dims) == sizeof...(Indices))
             && (std::is_same_v<size_t, Indices> && ...))
  {
    return Base::at(toIdx(indices...));
  }

  template<typename... Indices>
  auto& operator[](const Indices... indices) noexcept
    requires((sizeof...(dims) > 1) && (sizeof...(dims) == sizeof...(Indices))
             && (std::is_same_v<size_t, Indices> && ...))
  {
    return Base::at(toIdx(indices...));
  }

  template<typename... Indices>
  const auto& operator[](const Indices... indices) const noexcept
    requires((sizeof...(dims) > 1) && (sizeof...(dims) == sizeof...(Indices))
             && (std::is_same_v<size_t, Indices> && ...))
  {
    return Base::at(toIdx(indices...));
  }

 private:
  template<typename... Indices>
  static constexpr size_t toIdx(const Indices... indices) noexcept
    requires((sizeof...(dims) == sizeof...(Indices)) && (std::is_same_v<size_t, Indices> && ...))
  {
    return MDArrayDetail::DimHelper<dims...>::toIdx(indices...).index;
  }
};

int main() {
  {
    MDArray<int, 3, 3, 3> arr3d;

    std::iota(std::begin(arr3d), std::end(arr3d), 0);

    for (const auto iz : {0uz, 1uz, 2uz}) {
      for (const auto iy : {0uz, 1uz, 2uz}) {
        for (const auto ix : {0uz, 1uz, 2uz}) {
          std::cout << std::format("{:3}", arr3d[ix, iy, iz]);
        }
        std::cout << '\n';
      }
      std::cout << '\n';
    }
  }

  {
    MDArray<int, 7, 3> arr2d;

    std::iota(std::begin(arr2d), std::end(arr2d), 0);

    for (const auto iy : {0uz, 1uz, 2uz}) {
      for (const auto ix : {0uz, 1uz, 2uz, 3uz, 4uz, 5uz, 6uz}) {
        std::cout << std::format("{:3}", arr2d[ix, iy]);
      }
      std::cout << '\n';
    }
    std::cout << '\n';
  }

  {
    MDArray<int, 4> arr1d;

    std::iota(std::begin(arr1d), std::end(arr1d), 0);

    for (const auto ix : {0uz, 1uz, 2uz, 3uz}) {
      std::cout << std::format("{:3}", arr1d[ix]);
    }
    std::cout << '\n';
  }

  return 0;
}
