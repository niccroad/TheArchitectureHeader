#ifndef THE_ARCHITECTURE_H
#define THE_ARCHITECTURE_H

/**
 * Architecture (and compile time Mocking)
 * ---------------------------------------
 * This header file supports implementation of an architectural pattern for building C++ systems.
 * Related to this it also supports easy compile time mocking of architectural units.
 *
 * Usage instructions:
 *     * Copy this header into your code base. You will need to configure it for your project.
 *       Configure it according to the TODO comment below.
 *     * This header should be included everywhere. You should probably add it to all compiles
 *       using a command line flag in this case. Alternatively include it in a
 *       precompiled-header.
 *     * Do not define ARCH_COMPILING_TESTS when building your main code-base.
 *     * Add units of the code base to architecturally significant layers using the
 *       ARCH_NAMESPACE macro.
 *     * Access these units via the identifier given to the ARCH_NAMESPACE macro.
 *
 * Usage in unit-test builds:
 *     * Add the compiler #define ARCH_COMPILING_TESTS to the command line for all unit-test
 *       translation units.
 *     * You can still link against the main system objects which have been compiled without
 *       this #define.
 *
 * Original work Copyright (c) 2018 Nicolas Croad
 * Modified work Copyright (c) [COPYRIGHT HOLDER]
 */

#define ARCH_IMPL_SUFFIX             _impl
#define ARCH_TEST_SUFFIX             _test

#define JOIN_NAMES(tokenA, tokenB)                 tokenA ## tokenB
#define JOIN_NAMES_2(tokenA, tokenB)               JOIN_NAMES(tokenA, tokenB)

#ifndef ARCH_COMPILING_TESTS

// Add the implementation to the actual namespace.
#define ADD_IMPLEMENTATION(layer)                                \
    namespace JOIN_NAMES_2(layer, ARCH_IMPL_SUFFIX) { }          \
    namespace layer {                                            \
        using namespace JOIN_NAMES_2(layer, ARCH_IMPL_SUFFIX) ;  \
    }

// TODO: Configure your layers here. You may also want to
//       comment on what kinds of code should inhabit a
//       particular layer.

// Code in the gateway namespace should be where the serialization
// and de-serialization to storage happens.
ADD_IMPLEMENTATION(Gateway);

// Code in the Entity namespace should be the most abstract
// and self contained objects of the system.
ADD_IMPLEMENTATION(Entity);

#undef ADD_IMPLEMENTATION

#endif // ARCH_COMPILING_TESTS

#ifndef ARCH_COMPILING_TESTS

/**
* This macro should be used to open an architecturally important namespace. Any
* valid C++ constructs can be wrapped this way, including classes/structs/unions
* or other namespaces. Anonymous namespaces should also be wrapped this way, to
* avoid duplicate symbols in compile-time-mock unit-tests. If you do link your
* implementation code (the translation units compiled without #define ARCH_COMPILING_TESTS)
* against your unit-tests you can still access the non-compile-time-mock code in the
* namespace provided to ARCH_NAMESPACE suffixed with _impl directly.
*
* Usage example,
*
* ------ StoreEntityToFile.h ------
*     ARCH_NAMESPACE(Gateway) {
*         void storeToFile(const std::string& fileName, Entity::someEntity e);
*     }
*
* ------ StoreEntityToFile.cpp ----
*     ARCH_NAMESPACE(Gateway) {
*         void storeToFile(const std::string& fileName, Entity::someEntity e) {
*             // Implementation of storing entity to file goes here...
*         }
*     }
*/
#define ARCH_NAMESPACE(layer)               namespace JOIN_NAMES_2(layer, ARCH_IMPL_SUFFIX)

#else

#define ARCH_NAMESPACE(layer)               namespace JOIN_NAMES_2(layer, ARCH_TEST_SUFFIX)

#define TEST_MOCK_NAMESPACE(layer)                                 \
    namespace layer {                                              \
        namespace

/**
 * Inside a test translation unit wrap mock versions of some components
 * in MOCK_NAMESPACE and END_MOCK_NAMESPACE blocks. Then include the translation
 * unit (yes, the .cpp file) containing the code under test. Example usage,
 *
 * ------ StoreToGatewayTest.cpp ----------
 *     int numberOfEntitiesStored;
 *     MOCK_NAMESPACE(Gateway) {
 *         void storeToFile(const std::string& fileName, Entity::someEntity e) {
 *             EXPECT_EQ("DefaultFile.dat", fileName);
 *             numberOfEntitiesStored++;
 *         }
 *     } END_MOCK_NAMESPACE
 *     #include "StoreToGateway.cpp"
 *     TEST(TestStoreToGateway) {
 *         numberOfEntitiesStored = 0;
 *         Entity e;
 *         Interactor_test::addNewEntity(e);
 *         EXPECT_EQ(1, numberOfEntitiesStored);
 *     }
 */
#define MOCK_NAMESPACE(layer)          TEST_MOCK_NAMESPACE(layer)
#define END_MOCK_NAMESPACE             }

#endif // ARCH_COMPILING_TESTS

#endif // THE_ARCHITECTURE_H
