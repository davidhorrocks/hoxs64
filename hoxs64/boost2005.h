#include <memory>
#include <list>
#include <map>
#if (_MSC_VER < 1600)
#include <boost/smart_ptr.hpp>
using boost::shared_ptr;
using boost::weak_ptr;
#else
using std::shared_ptr;
using std::weak_ptr;
#endif
