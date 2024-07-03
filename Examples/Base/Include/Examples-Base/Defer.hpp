template <typename F>
struct Defer {
	Defer(F f) : f(f) {}
	~Defer() { f(); }
	F f;
};

template <typename F>
Defer<F> defer_create(F f) {
	return Defer<F>(f);
};

#define defer__(line) defer_ ## line
#define defer_(line) defer__(line)

struct DeferDummy {};

template<typename F>
Defer<F>operator+ (DeferDummy, F&& f)
{
	return defer_create<F>(std::forward<F>(f));
}

#define re_defer auto defer_(__LINE__) = DeferDummy() + [&]()