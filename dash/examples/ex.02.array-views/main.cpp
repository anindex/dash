#include <libdash.h>

using std::cout;
using std::cerr;
using std::clog;
using std::cin;
using std::endl;
using std::vector;


#define print(stream_expr__) \
  do { \
    std::ostringstream ss; \
    ss   << "[unit: " << dash::myid() << "] "; \
    ss   << stream_expr__ << endl; \
    cout << ss.str(); \
  } while(0)


template <class ValueRange>
static std::string range_str(
  const ValueRange & vrange) {
  typedef typename ValueRange::value_type value_t;
  std::ostringstream ss;
  auto idx = dash::index(vrange);
  int        i   = 0;
  for (const auto & v : vrange) {
    ss << std::setw(2) << *(dash::begin(idx) + i) << "|"
       << std::fixed << std::setprecision(4)
       << static_cast<const value_t>(v) << " ";
    ++i;
  }
  return ss.str();
}

template <class ArrayT>
auto initialize_array(ArrayT & array)
-> typename std::enable_if<
              std::is_floating_point<typename ArrayT::value_type>::value,
              void >::type
{
  auto block_size = array.pattern().blocksize(0);
  for (auto li = 0; li != array.local.size(); ++li) {
    auto block_lidx = li / block_size;
    auto block_gidx = (block_lidx * dash::size()) + dash::myid().id;
    auto gi         = (block_gidx * block_size) + (li % block_size);
    array.local[li] = // unit
                      (1.0000 * dash::myid().id) +
                      // local offset
                      (0.0100 * (li+1)) +
                      // global offset
                      (0.0001 * gi);
  }
  array.barrier();
}

int main(int argc, char *argv[])
{
  using namespace dash;

  dash::init(&argc, &argv);

  int elem_per_unit    = 5;
  int elem_additional  = 2;
  int array_size       = dash::size() * elem_per_unit +
                           std::min<int>(elem_additional, dash::size());

  dash::Array<float> a(array_size, dash::BLOCKCYCLIC(3));
  initialize_array(a);

  dash::barrier();
  if (dash::myid() == 0) {
    print("array: " << range_str(a));
  }

  auto l_array = a | local();
  print("array | local(): " << range_str(l_array));

  dash::barrier();

  auto copy_num_elem       = a.size() / 2;
  auto copy_dest_begin_idx = a.size() / 4;
  auto copy_dest_end_idx   = copy_dest_begin_idx + copy_num_elem;

  std::vector<float> buf(copy_num_elem);
  std::iota(buf.begin(), buf.end(), 0.9999);

  a.barrier();

  if (dash::myid() == 0) {
    print("copy target index range: "
            << "[" << copy_dest_begin_idx
            << "," << copy_dest_end_idx << ")");

    auto copy_begin_it   = a.begin() + copy_dest_begin_idx;
    auto copy_end_it_exp = copy_begin_it + copy_num_elem;

    auto dest_range      = dash::make_range(copy_begin_it,
                                            copy_end_it_exp);
    auto dest_blocks     = dash::blocks(dest_range);

    // Printing temporaries from view expressions instead of
    // named values for testing:
    print("target index set:  " << dash::index(dest_range));
    print("target block set:  " << dash::index(dash::blocks(dest_range)));
    print("copy target range: " << range_str(dest_range));

    for (const auto & block : dest_blocks) {
      print("copy to block: " << range_str(block));
    }

    // copy local buffer to global array
    auto copy_end_it     = dash::copy(
                             buf.data(),
                             buf.data() + copy_num_elem,
                             copy_begin_it);
  }
  a.barrier();

  print("modified array: " << range_str(a));

  dash::finalize();
}
