#include <iostream>
//--------------------------------------------------------------------------
// A helper to access parent methods context without passing it 
// as a parameter to inner methods
// Usage:
//      Some entry point -
//          class SomeContextClass : public stacky<SomeContextClass> {...}
//          SomeContextClass contextInstance;
//      Or 
//          class SomeContextClass {...}
//          SomeContextClass contextInstance;
//          stacky<SomeContextClass> scopeScopeHolder{contextInstance};
// 
//      Somewhere deep inside -
//          stacky<SomeContextClass>::get()->accessContextInstanceMethod();
// 
//--------------------------------------------------------------------------
template <class T> class stacky
{
private:
    stacky* prev; //holds linked list of stacky instances in stack
    T* contextPointer;
    // for mixed-in constructors like class SomeContextClass : public stacky<SomeContextClass> {...}
    explicit stacky() : stacky(static_cast<T&> (*this))
    {
    }
    //avoiding CRTP abuse so only derived friend class can call our private default constructor
    friend T;
public:
    explicit stacky(T& t) : contextPointer(&t)
    {
        stacky*& onTop = top();
        prev = onTop;
        onTop = this;
    }
    static T* get()
    {
        stacky*& onTop = top();
        T* t = onTop ? onTop->contextPointer : nullptr;
        return t;
    }
    ~stacky()
    {
        top() = prev;
    }
    stacky(stacky&) = delete;
    stacky& operator=(const stacky&) = delete;
    stacky(stacky&&) noexcept = delete;
    stacky const& operator=(stacky&&) = delete;
private:
    static stacky*& top()
    {
        static thread_local stacky* onTop{ nullptr };
        return onTop;
    }
};

//--------------------------------------------------------------------------
// Let's test
//--------------------------------------------------------------------------
class LoggingContext : public stacky<LoggingContext> { //use mixin(CRTP) 
    bool isEnabled = false;
public:
    void enabled(bool contextPointer) { isEnabled = contextPointer; }
    void log(const char* s) const { if (isEnabled) std::cout << s << std::endl; }
};
class StatisticsContext { //no mixin
    int count = 0;
public:
    void increment() { ++count; }
    void dump() const { std::cout << "count=" << count << std::endl; }
};
class FoolProof : public stacky< StatisticsContext > {};//This will not work
void innerFunction();
void middleFunction();
void outerFunction();

int main()
{
    LoggingContext loggingContext; //mixin makes it automatically available to child methods
    StatisticsContext statisticsContext; //no mixin - need to explicitly register via the following:
    stacky<StatisticsContext> statisticsContextScope{ statisticsContext };
    //FoolProof foolProof;     //--> Compile error - wrong mixin in class declaration
    //stacky<LoggingContext> s;//--> Compile error - cannot construct without derived class instance
    []() { outerFunction(); }(); 
    statisticsContext.dump();
    //"Enabled message" "count=3" will be printed
    return 0;
}
void outerFunction() {
    stacky<LoggingContext>::get()->log("Disabled message");
    middleFunction();
    stacky<StatisticsContext>::get()->increment();
}
void middleFunction() {
    stacky<LoggingContext>::get()->enabled(true);
    innerFunction();
    stacky<StatisticsContext>::get()->increment();
}
void innerFunction() {
    stacky<LoggingContext>::get()->log("Enabled message");
    stacky<StatisticsContext>::get()->increment();
}
