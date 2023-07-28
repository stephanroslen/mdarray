#include <array>
#include <cassert>
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
  constexpr auto& at(const Indices... indices) noexcept
    requires((sizeof...(dims) > 1) && (sizeof...(dims) == sizeof...(Indices))
             && (std::is_same_v<size_t, Indices> && ...))
  {
    return Base::at(toIdx(indices...));
  }

  template<typename... Indices>
  constexpr const auto& at(const Indices... indices) const noexcept
    requires((sizeof...(dims) > 1) && (sizeof...(dims) == sizeof...(Indices))
             && (std::is_same_v<size_t, Indices> && ...))
  {
    return Base::at(toIdx(indices...));
  }

  template<typename... Indices>
  constexpr auto& operator[](const Indices... indices) noexcept
    requires((sizeof...(dims) > 1) && (sizeof...(dims) == sizeof...(Indices))
             && (std::is_same_v<size_t, Indices> && ...))
  {
    return Base::at(toIdx(indices...));
  }

  template<typename... Indices>
  constexpr const auto& operator[](const Indices... indices) const noexcept
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
    (assert(indices < dims), ...);
    return MDArrayDetail::DimHelper<dims...>::toIdx(indices...).index;
  }
};

template<size_t... dims>
constexpr auto genIntArray() noexcept {
  MDArray<int, dims...> arr;

  std::iota(std::begin(arr), std::end(arr), 0);

  return arr;
}

template<size_t... dims>
void printIntArray(const MDArray<int, dims...>& arr) noexcept {
  constexpr static size_t nDims{sizeof...(dims)};
  using DimArr = std::array<size_t, nDims>;

  constexpr static DimArr dimVals{dims...};
  constexpr static DimArr zeroIter{[]() {
    DimArr result;
    std::fill(std::begin(result), std::end(result), size_t{0});
    return result;
  }()};

  const auto incIter{[&](DimArr& iter) {
    iter[0] += 1;
    for (size_t i{0}; i < nDims - 1; ++i) {
      if (iter[i] < dimVals[i]) {
        break;
      }
      iter[i] = 0;
      iter[i + 1] += 1;
    }
  }};

  for (auto iter{zeroIter}; iter[nDims - 1] < dimVals[nDims - 1]; incIter(iter)) {
    std::cout << std::format("{:3}", std::apply([&](auto... args) { return arr.at(args...); }, iter));
    if (iter[0] == dimVals[0] - 1) {
      std::cout << '\n';
      if (nDims > 1 && iter[1] == dimVals[1] - 1) {
        std::cout << '\n';
      }
    }
  }
}

int main() {
  {
    constexpr auto arr3d{genIntArray<3, 3, 3>()};

    printIntArray(arr3d);
  }

  {
    constexpr auto arr2d{genIntArray<7, 3>()};

    printIntArray(arr2d);
  }

  {
    constexpr auto arr1d{genIntArray<4>()};

    printIntArray(arr1d);
  }

  return 0;
}
