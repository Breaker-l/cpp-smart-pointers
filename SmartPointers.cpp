#include "SharedPtr.h"
#include "UniquePtr.h"
#include "WeakPtr.h"

#include <iostream>
#include <string>
#include <utility>

struct Widget {
    static int alive;

    int id = 0;

    Widget() { ++alive; }
    explicit Widget(int value) : id(value) { ++alive; }
    ~Widget() { --alive; }
};

int Widget::alive = 0;

struct CountingDeleter {
    int* counter;

    void operator()(int* p) const {
        ++*counter;
        delete p;
    }
};

int failures = 0;

void check(bool ok, const char* name) {
    std::cout << (ok ? "[OK]   " : "[FAIL] ") << name << '\n';
    if (!ok) {
        ++failures;
    }
}

void demoUnique() {
    std::cout << "--- UniquePtr ---\n";

    {
        UniquePtr<int> p(new int(42));
        check(*p == 42, "value");
        check(p.get() != nullptr, "get");
        check(static_cast<bool>(p), "operator bool");

        UniquePtr<int> q(std::move(p));
        check(p.get() == nullptr, "move clears source");
        check(*q == 42, "move keeps value");

        int* raw = q.release();
        check(raw != nullptr && *raw == 42, "release");
        check(!q, "release clears");
        delete raw;
    }

    {
        UniquePtr<int> p(new int(1));
        p.reset(new int(2));
        check(*p == 2, "reset");
        p.reset();
        check(p.get() == nullptr, "reset null");
    }

    {
        int deleted = 0;
        {
            UniquePtr<int, CountingDeleter> p(new int(9), CountingDeleter{ &deleted });
            UniquePtr<int, CountingDeleter> q(std::move(p));
            check(*q == 9, "stateful deleter after move");
        }
        check(deleted == 1, "deleter moved with pointer");
    }

    {
        UniquePtr<Widget> w(new Widget(7));
        check(Widget::alive == 1, "object alive");
        check(w->id == 7, "operator->");
    }
    check(Widget::alive == 0, "object destroyed");
}

void demoShared() {
    std::cout << "--- SharedPtr ---\n";

    {
        SharedPtr<Widget> sp(new Widget(1));
        check(sp.useCount() == 1, "useCount 1");

        SharedPtr<Widget> sp2 = sp;
        check(sp.useCount() == 2, "copy increments");
        check(sp2->id == 1, "operator->");

        SharedPtr<Widget> sp3;
        sp3 = sp;
        check(sp.useCount() == 3, "assignment increments");

        sp2.reset();
        sp3.reset();
        check(sp.useCount() == 1, "reset decrements");

        sp.reset();
        check(Widget::alive == 0, "object destroyed");
    }
}

void demoWeak() {
    std::cout << "--- WeakPtr ---\n";

    SharedPtr<Widget> sp(new Widget(3));
    WeakPtr<Widget> wp(sp);

    check(!wp.expired(), "alive");
    check(wp.useCount() == 1, "useCount");

    {
        SharedPtr<Widget> locked = wp.lock();
        check(locked.get() != nullptr, "lock returns object");
        check(locked->id == 3, "lock value");
        check(locked.useCount() == 2, "lock increments");
    }

    SharedPtr<Widget> keep = wp.lock();
    sp.reset();
    check(!wp.expired(), "lock keeps object alive");
    keep.reset();
    check(wp.expired(), "expired after owner gone");
    check(wp.lock().get() == nullptr, "lock empty when expired");

    WeakPtr<Widget> copy = wp;
    WeakPtr<Widget> moved(std::move(copy));
    check(copy.expired(), "moved-from weak is empty");
    check(moved.expired(), "moved weak expired");

    WeakPtr<Widget> empty;
    check(empty.expired(), "default weak empty");
    check(Widget::alive == 0, "object destroyed");
}

void demoArrays() {
    std::cout << "--- arrays ---\n";

    {
        UniquePtr<int[]> u = makeUnique<int[]>(4);
        for (std::size_t i = 0; i < 4; ++i) {
            u[i] = static_cast<int>(i * i);
        }
        std::cout << "unique array:";
        for (std::size_t i = 0; i < 4; ++i) {
            std::cout << ' ' << u[i];
        }
        std::cout << '\n';
    }

    {
        SharedPtr<int[]> s(new int[3] {10, 20, 30});
        std::cout << "shared array:";
        for (std::size_t i = 0; i < 3; ++i) {
            std::cout << ' ' << s[i];
        }
        std::cout << '\n';

        SharedPtr<int[]> s2 = s;
        check(s.useCount() == 2, "shared array copy");
    }

    {
        UniquePtr<std::string> text = makeUnique<std::string>("hello");
        check(*text == "hello", "makeUnique string");
        std::cout << "makeUnique string: " << *text << '\n';
    }
}

int main() {
    demoUnique();
    demoShared();
    demoWeak();
    demoArrays();

    std::cout << "\nfailures: " << failures << '\n';
    return failures == 0 ? 0 : 1;
}