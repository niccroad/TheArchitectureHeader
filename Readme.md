Software Architecture, Decoupling (and mock unit-testing)
=========================================================

This single header file C++ project enables implementation of software architecture,
dependency inversion (at compile time), and mock based unit testing. It attempts to
do so despite eliminating much of the code typically associated with software containing
heavy use of the dependency inversion pattern.

Usage
-----
Detailed usage instructions are provided in the header file. My suggestion is to copy it
into your project and further modify it as required from there.

What kind of architecture
-------------------------
The primary style of architecture motivating this usage is called an Onion or Hexagonal
software architecture (though other styles may be supported). In this case the more stable
(less subject to change) and abstract parts of the software inhabit the inner (lower) layers
of the software and the more unstable (more subject to change) and concrete parts of the
software inhabit the outer (upper) layers. It's usually made a requirement for dependencies
to only run from the outer layers towards the inner layers and so implementing such a system
will make regular use of the dependency inversion pattern to make these dependencies be
correctly directed.
One of the benefits of such an architecture is that the components of the system can be easily
unit-tested, often by mocking the more concrete components utilised by the implementation of
the more abstract components.

What is the dependency inversion pattern
----------------------------------------
The dependency inversion pattern is a reasonably simple and often repeated pattern in software.
It exists in some form in virtually all programming languages. In C++ we may use it when finding
a class which depends on (e.g needs to include the header for) another class which we want to be
a more concrete level.

**MoreConcrete.h**
```cpp
class MoreConcrete {
public:
    void doThing() {
        // Do the thing!
    }

    void doAnotherThing() {
        // Do some other thing!
    }
};
```
**MoreAbstract.h**
```cpp
#include "MoreConcrete.h"
class MoreAbstract {
public:
    void method(MoreConcrete concrete) {
        concrete.doThing();
    }
};
```

So if we want to re-factor this so the dependencies run from the more abstract towards the
more concrete we might use an interface by refactoring this to be as follows,

**ConcreteInterface.h**
```cpp
class ConcreteInterface {
public:
    virtual void doThing() = 0;
    virtual void doAnotherThing() = 0;
};
```
**MoreAbstract.h**
```cpp
#include "ConcreteInterface.h"
class MoreAbstract {
public:
    void method(ConcreteInterface& concrete) {
        concrete.doThing();
    }
};
```
**MoreConcrete.h**
```cpp
#include "ConcreteInterface.h"
class MoreConcrete : public ConcreteInterface {
public:
    void doThing() override {
        // Do the thing!
    }

    void doAnotherThing() override {
        // Do some other thing!
    }    
};

Now the dependencies point in the desireable direction. One upshot of this is that in order
to unit-test the MoreAbstract class we can pass it a mock implementation of the
ConcreteInterface interface and so write the unit-test to examine only the implementation of
method and so avoid it needing to be effected by the implementation of (or changes to the 
implementation of) our doThing method.

What is the dependency injection pattern
----------------------------------------
Another related pattern is the dependency injection pattern. This one primarily facilitates
unit-testing of components. The idea is relatively simple you provide concrete dependencies
required by a more abstract implementation, injected as parameters. In this example we will
deal with an implementation of something which might want to access time information from
the operating system clock. To separate this example from the prior one I used an example
not using runtime polymorphism here.

**SystemClock.h**
```cpp
class SystemClock {
public:
    SystemClock(time_t fixedTime = 0)
      : isFixed(fixedTime != 0),
        theTime(fixedTime) {
    }

    time_t getTheTime() {
        if (isFixed) { return theTime; }
        return std::time();
    }

private:
    bool isFixed = false;
    time_t theTime;
};
```
**TimerStrategy.h**
```cpp
class TimerStrategy {
public:
    TimerStrategy(time_t time)
      : nextTriggerTime(time)
    {
    }

    bool hasTriggered() {
        SystemClock clock;
        bool triggered = hasFired || clock.getTheTime() >= nextTriggerTime;
        hasFired = triggered;
        return triggered;
    }
private:
    bool hasFired = false;
    time_t nextTriggerTime;
};
```

Initially we might implement the TimerStrategy class as follows. This works ok until it comes
to testing the hasTriggered method as in this case we are depending on the system clock time
for the implementation of our test and have no way to change the value the system clock
returns in this case.

**TimerStrategy.h**
```cpp
class TimerStrategy {
public:
    TimerStrategy(time_t time)
      : nextTriggerTime(time)
    {
    }

    bool hasTriggered(SystemClock clock) {
        bool triggered = hasFired || clock.getTheTime() >= nextTriggerTime;
        hasFired = triggered;
        return triggered;
    }
private:
    bool hasFired = false;
    time_t nextTriggerTime;
};
```
**TimerStrategyTest.cpp**
```cpp
TEST(TimerStrategyTest, TestStrategyHasTriggeredOnTime) {
    TimerStrategy strategy(15000000);

    SystemClock earlier(14543563);
    EXPECT_FALSE(strategy.hasTriggered(earlier));

    SystemClock later(15872834);
    EXPECT_TRUE(strategy.hasTriggered(later));
}
```

So here we can see the dependency on the system clock is injected into the hasTriggered method
allowing this to be unit-tested.

What are the advantages
-----------------------
I am going to suggest the need for these patterns boils down to two significant benefits,

**Several strengths**
* It's possible to unit-test the components.

   So we are able to add test's just checking over the code of a particular component largely
   isolated from the rest of the system. This allows the test to check just the components
   code and limits the need for the tests to change when other components change behaviour.

* The system's interactions with interfaces like ConcreteInterface are listed in specific API's.

   This one is particularly important when dealing with things like databases or external
   frameworks. The benefit being that these interactions (things like SQL statements) are
   limited to a smaller number of parts of the software and the expections should be well
   defined for such interactions (e.g due to the interface method names).

So what's the problem(s) then?
------------------------------
There are actually a few weaknesses which this pattern suffers from, which I will go through.
On the other hand it should be highlighted your probably much better off using this pattern
in programming languages where there is no alternative. Not being able to unit-test significant
parts of a large program is a much greater problem than any weaknesses presented here.

**Several pattern weaknesses**
* The pattern limits the implementation to a specific subset of C++ language features.

   The implementation above uses runtime polymorphism, there is another implementation
   in c++ using class templates. However if your implementing this dependency inversion
   pattern then certain natural language features such as using free-functions or static
   variables will not be able to be used.

* The surface area of the code base is increases (often in highly repetitive ways).

   For the runtime polymorphism based implementation of this pattern then the interface
   of the MoreConcrete class above needs to be declared twice. Once in ConcreteInterface
   and again in MoreConcrete. Typically all the public methods of MoreConcrete are
   declared twice, once in pure virtual form and a second time in their implementation. An
   additional drawback of this is that frequently it will be convenient to mock this interface
   for the purposes of some unit-test, but with only a few methods of a wider interface being needed for a specific test. In this case all the pure virtual methods will still need to be
   defined though they may never be invoked. This mock class will likely need to be further updated when this interface changes.

* The encapsulation of the more abstract class implementation may be exposed un-desirably.

   One way we think about encapsulation in C based languages is in terms of which headers
   need to be included to compile any translation unit, and also what needs to be
   re-compiled on a change to any header particular file. In the case that the MoreConcrete
   class is not a necessary part of the interface of the MoreAbstract class then it will
   need to be provided to this interface, and at minimum forward declared in (if not included
   by) MoreAbstract.h. This is typically the case for certain dependencies on things such
   as operating system functions for which it is frequently desirable to dependency inject
   these during unit-testing. This dependency on the ConcreteInterface is still a weakness
   however and may cause a significant number of translation units and tests to be re-compiled
   (implying coupling) when the interface in ConcreteInterface.h needs to be modified.

* Additional dependencies are frequently added by these patterns.

   Again thinking about which headers need to be included. For certain components we would
   prefer only to include their headers into the compilation of the translation units which
   make use of them. A good example of this might be an interface to operating system
   functionality. With extensive use of dependency inversion and dependency injection then
   often the interface header of the operating system needs to be included (or a forward
   declaration provided) in many places which have only a tangental relationship with
   operating system functionality.

* There is frequently runtime overhead added by these patterns.

   I am dealing with C++ so of course this is a frequent concern to the use of this pattern.
   In many cases this concern is probably exaggerated however use of runtime polymorphism and
   virtual function calls adds a significant overhead over the non-virtual function alternatives
   in C++.

* There is frequently compile time overhead added by these patterns.

   When using the template function implementation of this pattern then this code will require
   more time to build.

What is the alternative?
------------------------
The alternative I propose here I have called compile time dependency inversion. The idea is
very simple, we will compile the code with compile time dependencies between the users and
actual component implementations and when writing unit tests we will instead compile the
code under test against mock implementations. The implementation details to achieve this
are easy to enough to observe from the header file itself. Also importantly the mechansims
used have been a part of even the C++98 standard.

The earlier example of dependency inversion may now be written as follows,

**MoreConcrete.h**
```cpp
#include "ConcreteInterface.h"
ARCH_NAMESPACE(ConcreteThings) {
    class MoreConcrete {
    public:
        void doThing() {
            // Do the thing!
        }

        void doAnotherThing() {
            // Do some other thing!
        }
    };
}
```
**MoreAbstract.h**
```cpp
#include "ConcreteInterface.h"
ARCH_NAMESPACE(AbstractThings) {
    class MoreAbstract {
    public:
        void method(ConcreteThings::MoreConcrete concrete) {
            concrete.doThing();
        }
    };
}
```

And amended with an example unit-test.

**MoreAbstractTest.cpp**
```cpp
MOCK_NAMESPACE(ConcreteThings) {
    class MoreConcrete {
    public:
        void doThing() {
            numberOfTimesThingWasDone++;
        }

        int numberOfTimesThingWasDone = 0;    
    };
} END_MOCK_NAMESPACE
// Note, if the implementation was in a .cpp file we would include that here instead.
#include "MoreAbstract.h"

TEST(MoreAbstractTest, TestTheMethod) {
    ConcreteThings::MoreConcrete c;
    AbstractThings_test::MoreAbstract a;
    a.method(c);
    EXPECT_EQ(1, c.numberOfTimesThingWasDone);
}
```

Why is that better?
-------------------
This has the following advantages over the original implementation,

* The implementation was able to use the most natural C++ language features.

   In this case it was natural to have a non-virtual class passed as a parameter, by
   value. In other cases we can use language features such as free-functions,
   constructor calls. In fact the full compliment of C++ language features can be
   used and still mocked as needed.

* The interface didn't need to be repeated.

   No abstract interface was needed to mock this example. Typically this involves simply
   repeating the declarations already made once in MoreConcrete.h in pure virtual form.

* Functions not needed by the unit-test didn't need to be declared.

   The method doAnotherThing() was not needed to implement this test. This meant no empty
   implementation of that method as pure virtual was needed anywhere and this was not written
   anywhere. Further if that methods interface ever changes of other methods are added this
   unit-test does not need to be updated.

* There was no added runtime overhead.

   Runtime polymorphism was not needed and no runtime overhead was added by using it.

Are the same advantages still present
-------------------------------------
Essentially, yes, I think so.

**Several strengths**
* It's possible to unit-test the components.

   It's still possible to unit-test the components. In fact we have made that significantly
   easier to do and requiring less refactoring to make the code unit-testable.

* The system's interactions with interfaces like MoreConcrete are listed in specific API's.

   The question on if this benefit was retained depends on if we think there is value to
   the methods on MoreConcrete being re-declared for the interface ConcreteInterface. The
   only benefit I can see to repeating this is in the event of the system changing to
   require multiple runtime polymorphic versions of ConcreteInterface. The benefit here
   being that we already wrote the interface ConcreteInterface and don't need to take this
   step in order to make that refactoring. But of course the downside is that if this
   never eventuates as a useful change then we have implemented an un-necessary over
   generalisation of this type. Essentially I don't think there is benefit to this kind of
   early (premature) refactoring of the code base. In the event this refactoring to make
   MoreConcrete into a runtime polymorphic type becomes pertinent this would seem to be
   the best time to implement this change anyway.
   
Copyright (c) 2018 Nicolas Croad
