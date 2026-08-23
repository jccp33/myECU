#include "signal_store.hpp"

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace {

bool expectTrue(const char* testName, bool condition) {
    if (!condition) {
        std::cerr << "FAILED: " << testName << '\n';
        return false;
    }

    std::cout << "PASSED: " << testName << '\n';
    return true;
}

SignalSample createSample(
    const SignalId& id,
    float value = 10.0F,
    TimestampMs timestampMs = 100U,
    SignalValidity validity = SignalValidity::VALID
) {
    return SignalSample(id, value, timestampMs, validity);
}

bool testNewStoreIsEmpty() {
    const SignalStore store;
    return expectTrue(
        "new signal store is empty",
        store.size() == 0U
            && store.capacity() == MAX_SENSOR_COUNT
            && store.get(0U) == nullptr
            && store.find(SignalId(1U, 1U, 1U, 0U)) == nullptr
    );
}

bool testInsertAndFindSample() {
    SignalStore store;
    const SignalId id(1U, 2U, 100U, 3U);
    const SignalStoreResult result = store.upsert(createSample(id, 42.5F, 500U));
    const SignalSample* stored = store.find(id);
    return expectTrue(
        "inserted sample can be found",
        result == SignalStoreResult::INSERTED
            && store.size() == 1U
            && stored != nullptr
            && stored->id == id
            && stored->value == 42.5F
            && stored->lastUpdateMs == 500U
            && stored->validity == SignalValidity::VALID
    );
}

bool testSignalIdentityUsesEveryField() {
    SignalStore store;
    const SignalId storedId(1U, 2U, 100U, 3U);
    store.upsert(createSample(storedId));

    const bool distinctIdsAreAbsent =
        store.find(SignalId(9U, 2U, 100U, 3U)) == nullptr
        && store.find(SignalId(1U, 9U, 100U, 3U)) == nullptr
        && store.find(SignalId(1U, 2U, 999U, 3U)) == nullptr
        && store.find(SignalId(1U, 2U, 100U, 9U)) == nullptr;

    return expectTrue(
        "signal lookup uses ECU source id and instance",
        distinctIdsAreAbsent
    );
}

bool testExistingSignalIsUpdatedWithoutDuplication() {
    SignalStore store;
    const SignalId id(1U, 2U, 100U, 0U);
    store.upsert(createSample(id, 10.0F, 100U));
    const SignalStoreResult result = store.upsert(
        createSample(id, 20.0F, 200U, SignalValidity::INVALID)
    );
    const SignalSample* stored = store.find(id);
    return expectTrue(
        "existing signal is updated without duplication",
        result == SignalStoreResult::UPDATED
            && store.size() == 1U
            && stored != nullptr
            && stored->value == 20.0F
            && stored->lastUpdateMs == 200U
            && stored->validity == SignalValidity::INVALID
    );
}

bool testStaleSampleIsRejectedWithoutModification() {
    SignalStore store;
    const SignalId id(1U, 2U, 100U, 0U);
    store.upsert(createSample(id, 20.0F, 200U));
    const SignalStoreResult result = store.upsert(createSample(id, 99.0F, 199U));
    const SignalSample* stored = store.find(id);
    return expectTrue(
        "stale sample is rejected without modifying stored value",
        result == SignalStoreResult::STALE_SAMPLE
            && store.size() == 1U
            && stored != nullptr
            && stored->value == 20.0F
            && stored->lastUpdateMs == 200U
    );
}

bool testEqualTimestampCanUpdateSample() {
    SignalStore store;
    const SignalId id(1U, 2U, 100U, 0U);
    store.upsert(createSample(id, 10.0F, 200U));
    const SignalStoreResult result = store.upsert(createSample(id, 11.0F, 200U));
    const SignalSample* stored = store.find(id);
    return expectTrue(
        "equal timestamp can update sample",
        result == SignalStoreResult::UPDATED
            && stored != nullptr
            && stored->value == 11.0F
            && stored->lastUpdateMs == 200U
    );
}

bool testGetUsesLogicalSize() {
    SignalStore store;
    const SignalId firstId(1U, 1U, 10U, 0U);
    const SignalId secondId(1U, 1U, 11U, 0U);
    store.upsert(createSample(firstId));
    store.upsert(createSample(secondId));

    const SignalSample* first = store.get(0U);
    const SignalSample* second = store.get(1U);
    return expectTrue(
        "indexed access is limited by logical size",
        first != nullptr
            && first->id == firstId
            && second != nullptr
            && second->id == secondId
            && store.get(2U) == nullptr
            && store.get(MAX_SENSOR_COUNT) == nullptr
    );
}

bool testSameNumericIdFromDifferentSourcesIsDistinct() {
    SignalStore store;
    const SignalId firstId(1U, 1U, 100U, 0U);
    const SignalId secondId(1U, 2U, 100U, 0U);
    const SignalStoreResult firstResult = store.upsert(createSample(firstId, 10.0F));
    const SignalStoreResult secondResult = store.upsert(createSample(secondId, 20.0F));
    return expectTrue(
        "same numeric id from different sources is distinct",
        firstResult == SignalStoreResult::INSERTED
            && secondResult == SignalStoreResult::INSERTED
            && store.size() == 2U
            && store.find(firstId) != nullptr
            && store.find(secondId) != nullptr
    );
}

bool testCapacityLimitIsEnforced() {
    SignalStore store;
    bool insertedAll = true;
    for (std::size_t index = 0U; index < MAX_SENSOR_COUNT; ++index) {
        const SignalId id(
            1U,
            1U,
            static_cast<std::uint16_t>(index),
            0U
        );
        if (store.upsert(createSample(id)) != SignalStoreResult::INSERTED) {
            insertedAll = false;
        }
    }

    const SignalStoreResult overflowResult = store.upsert(
        createSample(SignalId(1U, 1U, 1000U, 0U))
    );
    return expectTrue(
        "fixed signal capacity rejects one additional signal safely",
        insertedAll
            && store.size() == MAX_SENSOR_COUNT
            && overflowResult == SignalStoreResult::CAPACITY_EXCEEDED
            && store.size() == MAX_SENSOR_COUNT
    );
}

bool testClearAllowsStoreReuse() {
    SignalStore store;
    const SignalId oldId(1U, 1U, 10U, 0U);
    const SignalId newId(2U, 2U, 20U, 0U);
    store.upsert(createSample(oldId));
    store.clear();
    const SignalStoreResult result = store.upsert(createSample(newId));
    return expectTrue(
        "clear empties and allows signal store reuse",
        result == SignalStoreResult::INSERTED
            && store.size() == 1U
            && store.find(oldId) == nullptr
            && store.find(newId) != nullptr
    );
}

bool testSignalEnumsUseFixedWidthStorage() {
    const bool fixedWidth =
        std::is_same<
            std::underlying_type<SignalValidity>::type,
            std::uint8_t
        >::value
        && std::is_same<
            std::underlying_type<SignalStoreResult>::type,
            std::uint8_t
        >::value;
    return expectTrue("signal enums use uint8_t storage", fixedWidth);
}

}  // namespace

int main() {
    int failures = 0;
    const bool results[] = {
        testNewStoreIsEmpty(),
        testInsertAndFindSample(),
        testSignalIdentityUsesEveryField(),
        testExistingSignalIsUpdatedWithoutDuplication(),
        testStaleSampleIsRejectedWithoutModification(),
        testEqualTimestampCanUpdateSample(),
        testGetUsesLogicalSize(),
        testSameNumericIdFromDifferentSourcesIsDistinct(),
        testCapacityLimitIsEnforced(),
        testClearAllowsStoreReuse(),
        testSignalEnumsUseFixedWidthStorage()
    };

    const std::size_t count = sizeof(results) / sizeof(results[0]);
    for (std::size_t index = 0U; index < count; ++index) {
        if (!results[index]) {
            ++failures;
        }
    }

    if (failures != 0) {
        std::cerr << failures << " signal store test(s) failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "All signal store tests passed\n";
    return EXIT_SUCCESS;
}
