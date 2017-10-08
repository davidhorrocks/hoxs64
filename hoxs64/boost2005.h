#pragma once
#include <functional>
#include <memory>
#include <vector>
#include <list>
#include <map>
#if (_MSC_VER < 1600)
#include <boost/cstdint.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#pragma warning (disable : 4244)
#include <boost/random/mersenne_twister.hpp>
#pragma warning (default : 4244)
using boost::shared_ptr;
using boost::weak_ptr;
using boost::enable_shared_from_this;
using boost::static_pointer_cast;
using boost::function;
using boost::random_device;
using boost::mt19937;
using boost::random::uniform_int_distribution;
using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::int64_t;
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;
#else
#include <random>
using std::shared_ptr;
using std::weak_ptr;
using std::enable_shared_from_this;
using std::static_pointer_cast;
using std::function;
using std::random_device;
using std::mt19937;
using std::uniform_int_distribution;
#endif
