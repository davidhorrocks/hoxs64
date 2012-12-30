#include <functional>
#include <memory>
#include <vector>
#include <list>
#include <map>
#if (_MSC_VER < 1600)
#include <boost/enable_shared_from_this.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>
using boost::shared_ptr;
using boost::weak_ptr;
using boost::enable_shared_from_this;
using boost::static_pointer_cast;
using boost::function;
#else
using std::shared_ptr;
using std::weak_ptr;
using std::enable_shared_from_this;
using std::static_pointer_cast;
using std::function;
#endif
