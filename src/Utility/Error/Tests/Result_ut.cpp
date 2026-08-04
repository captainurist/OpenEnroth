#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "Testing/Unit/UnitTest.h"

#include "Utility/Error/Result.h"
#include "Utility/Exception.h"

static Result<int> parsePositive(int value) {
    if (value <= 0)
        return fail("'{}' is not a positive number", value);
    return value;
}

static Result<int> sumPositive(int l, int r) {
    int lv = co_await parsePositive(l);
    int rv = co_await parsePositive(r).withContext("while parsing the second argument");
    co_return lv + rv;
}

static Result<void> checkPositive(int value) {
    co_await parsePositive(value);
}

UNIT_TEST(Error, Message) {
    Error error("Cannot open '{}'", "foo.lod");
    EXPECT_EQ(error.message(), "Cannot open 'foo.lod'");
    EXPECT_EQ(error.outerMessage(), "Cannot open 'foo.lod'");
    EXPECT_EQ(error.innerMessage(), "Cannot open 'foo.lod'");
    EXPECT_FALSE(error.code());
}

UNIT_TEST(Error, ContextChaining) {
    Error inner("unexpected end of stream, requested {} bytes, got {}", 16, 4);
    Error middle = inner.withContext("cannot read the SND header");
    Error outer = middle.withContext("cannot open '{}'", "sounds.snd");

    EXPECT_EQ(outer.message(),
              "cannot open 'sounds.snd': cannot read the SND header: "
              "unexpected end of stream, requested 16 bytes, got 4");
    EXPECT_EQ(outer.outerMessage(), "cannot open 'sounds.snd'");
    EXPECT_EQ(outer.innerMessage(), "unexpected end of stream, requested 16 bytes, got 4");

    // Adding context doesn't mutate the original error.
    EXPECT_EQ(inner.message(), "unexpected end of stream, requested 16 bytes, got 4");
}

UNIT_TEST(Error, ErrorCodePropagatesThroughContext) {
    Error inner = Error::fromErrc(std::errc::no_such_file_or_directory, "foo.lod");
    Error outer = inner.withContext("cannot load the game data");

    EXPECT_EQ(outer.code(), std::make_error_code(std::errc::no_such_file_or_directory));
    EXPECT_TRUE(outer.message().starts_with("cannot load the game data: foo.lod: "));
}

UNIT_TEST(Error, Formatter) {
    Error error = Error("bad").withContext("very bad");
    EXPECT_EQ(fmt::format("[{}]", error), "[very bad: bad]");
}

UNIT_TEST(Error, FromCurrentException) {
    Error error = [] {
        try {
            throw std::system_error(std::make_error_code(std::errc::permission_denied), "nope");
        } catch (...) {
            return Error::fromCurrentException();
        }
    }();

    EXPECT_EQ(error.code(), std::make_error_code(std::errc::permission_denied));
    EXPECT_THAT(error.message(), testing::HasSubstr("nope"));
}

UNIT_TEST(Result, Success) {
    Result<int> result = parsePositive(10);
    ASSERT_TRUE(result);
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(*result, 10);
}

UNIT_TEST(Result, Failure) {
    Result<int> result = parsePositive(-1);
    ASSERT_FALSE(result);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.error().message(), "'-1' is not a positive number");
}

UNIT_TEST(Result, CoroutinePropagates) {
    Result<int> ok = sumPositive(1, 2);
    ASSERT_TRUE(ok);
    EXPECT_EQ(*ok, 3);

    Result<int> firstBad = sumPositive(-1, 2);
    ASSERT_FALSE(firstBad);
    EXPECT_EQ(firstBad.error().message(), "'-1' is not a positive number");

    Result<int> secondBad = sumPositive(1, -2);
    ASSERT_FALSE(secondBad);
    EXPECT_EQ(secondBad.error().message(), "while parsing the second argument: '-2' is not a positive number");
}

UNIT_TEST(Result, CoroutineVoidPropagates) {
    EXPECT_TRUE(checkPositive(1));

    Result<void> bad = checkPositive(0);
    ASSERT_FALSE(bad);
    EXPECT_EQ(bad.error().message(), "'0' is not a positive number");
}

UNIT_TEST(Result, WithContext) {
    Result<int> bad = parsePositive(-1).withContext("while doing '{}'", "something");
    ASSERT_FALSE(bad);
    EXPECT_EQ(bad.error().message(), "while doing 'something': '-1' is not a positive number");

    // Context on a successful result is a no-op.
    EXPECT_EQ(*parsePositive(1).withContext("unused"), 1);

    // Also works for Result<void>.
    Result<void> badVoid = checkPositive(0).withContext("outer");
    ASSERT_FALSE(badVoid);
    EXPECT_EQ(badVoid.error().message(), "outer: '0' is not a positive number");
}

UNIT_TEST(Result, ValueOr) {
    EXPECT_EQ(parsePositive(-1).valueOr(0), 0);
    EXPECT_EQ(parsePositive(5).valueOr(0), 5);
}

UNIT_TEST(Result, OrThrow) {
    EXPECT_EQ(parsePositive(3).orThrow(), 3);
    EXPECT_THROW_MESSAGE(parsePositive(-3).orThrow(), "positive");

    EXPECT_NO_THROW(checkPositive(3).orThrow());
    EXPECT_THROW_MESSAGE(checkPositive(-3).orThrow(), "positive");
}

UNIT_TEST(Result, TryCatch) {
    Result<int> thrown = tryCatch([] () -> int { throw Exception("oops {}", 1); });
    ASSERT_FALSE(thrown);
    EXPECT_EQ(thrown.error().message(), "oops 1");

    Result<int> fine = tryCatch([] { return 5; });
    ASSERT_TRUE(fine);
    EXPECT_EQ(*fine, 5);

    Result<void> voidFine = tryCatch([] {});
    EXPECT_TRUE(voidFine);
}

UNIT_TEST(Result, TryCatchDoesntDoubleWrap) {
    // A callable that returns a Result is passed through, not wrapped into a Result<Result<...>>. This is what
    // makes it OK to mix fail() with throwing calls inside one tryCatch block.
    Result<int> failed = tryCatch([] () -> Result<int> { return fail("nope"); });
    ASSERT_FALSE(failed);
    EXPECT_EQ(failed.error().message(), "nope");

    Result<int> thrown = tryCatch([] () -> Result<int> { throw Exception("oops"); });
    ASSERT_FALSE(thrown);
    EXPECT_EQ(thrown.error().message(), "oops");

    Result<int> fine = tryCatch([] () -> Result<int> { return 5; });
    ASSERT_TRUE(fine);
    EXPECT_EQ(*fine, 5);
}

UNIT_TEST(Result, MustSucceed) {
    EXPECT_EQ(parsePositive(3).mustSucceed(), 3);

    // Check that the fatal handler gets the message. We can't let `mustSucceed` actually terminate here, so the
    // handler longjmps out via an exception.
    struct FatalError {};
    static std::string reported;
    FatalErrorHandler oldHandler = setFatalErrorHandler([] (const Error &error) {
        reported = error.message();
        throw FatalError();
    });
    EXPECT_THROW(parsePositive(-3).mustSucceed(), FatalError);
    setFatalErrorHandler(oldHandler);
    EXPECT_EQ(reported, "'-3' is not a positive number");
}

UNIT_TEST(Result, Discard) {
    // `discard` is the explicit way to drop a Result. Mostly a compilation test - `parsePositive(-1);` alone
    // wouldn't compile because Result is [[nodiscard]].
    parsePositive(-1).discard();
    checkPositive(-1).discard();
}

static int coroGuardsDestroyed = 0;

namespace {
struct CoroGuard {
    ~CoroGuard() { coroGuardsDestroyed++; }
};
} // namespace

static Result<int> sumPositiveCoro(int l, int r) {
    CoroGuard guard; // Checks that locals are destroyed when an error aborts the coroutine.
    int lv = co_await parsePositive(l);
    int rv = co_await parsePositive(r);
    co_return lv + rv;
}

static Result<void> checkPositiveCoro(int value) {
    co_await parsePositive(value); // Coroutine awaiting a plain Result-returning function.
}

static Result<std::unique_ptr<int>> makeUniqueCoro(bool ok) {
    if (!ok)
        co_return fail("nope");
    co_return std::make_unique<int>(42);
}

static Result<int> useUniqueCoro(bool ok) {
    std::unique_ptr<int> value = co_await makeUniqueCoro(ok); // Coroutine awaiting a coroutine, move-only payload.
    co_return *value;
}

static Result<int> throwingCoro() {
    throw Exception("boom {}", 7); // Handled by the promise, becomes an Error.
    co_return 1; // Unreachable, but its presence is what makes this function a coroutine.
}

static Result<void> noCoReturnCoro() {
    co_await Result<void>();
    // No `co_return;` - a `Result<void>` coroutine's promise has `return_void`, so flowing off the end is
    // well-defined success, same as falling off the end of a plain `void` function.
}

static Result<void> coAwaitErrorCoro() {
    co_await fail("raised {}", 42); // Ends the coroutine right here.
    ADD_FAILURE() << "coAwaitErrorCoro resumed past a failed co_await.";
}

UNIT_TEST(Result, CoroutineSuccess) {
    coroGuardsDestroyed = 0;
    Result<int> ok = sumPositiveCoro(1, 2);
    ASSERT_TRUE(ok);
    EXPECT_EQ(*ok, 3);
    EXPECT_EQ(coroGuardsDestroyed, 1);

    EXPECT_TRUE(checkPositiveCoro(5));
}

UNIT_TEST(Result, CoroutineErrorShortCircuits) {
    coroGuardsDestroyed = 0;
    Result<int> bad = sumPositiveCoro(1, -2);
    ASSERT_FALSE(bad);
    EXPECT_EQ(bad.error().message(), "'-2' is not a positive number");
    EXPECT_EQ(coroGuardsDestroyed, 1); // Locals were destroyed when the failed co_await aborted the coroutine.
}

UNIT_TEST(Result, CoroutineReturnsFail) {
    Result<std::unique_ptr<int>> bad = makeUniqueCoro(false);
    ASSERT_FALSE(bad);
    EXPECT_EQ(bad.error().message(), "nope");
}

UNIT_TEST(Result, CoroutineAwaitsCoroutine) {
    Result<int> ok = useUniqueCoro(true);
    ASSERT_TRUE(ok);
    EXPECT_EQ(*ok, 42);

    EXPECT_FALSE(useUniqueCoro(false));
}

UNIT_TEST(Result, CoroutineCatchesExceptions) {
    Result<int> thrown = throwingCoro();
    ASSERT_FALSE(thrown);
    EXPECT_EQ(thrown.error().message(), "boom 7");
}

UNIT_TEST(Result, CoroutineFlowingOffTheEndIsSuccess) {
    EXPECT_TRUE(noCoReturnCoro().ok());
}

UNIT_TEST(Result, CoroutineCoAwaitError) {
    Result<void> raised = coAwaitErrorCoro();
    ASSERT_FALSE(raised.ok());
    EXPECT_EQ(raised.error().message(), "raised 42");
}

UNIT_TEST(Result, SizeIsSmall) {
    // `Error` is a single shared_ptr, so `Result<T>` doesn't blow up in size. This is what makes it viable to return
    // `Result`s from hot-ish code.
    EXPECT_EQ(sizeof(Error), sizeof(std::shared_ptr<void>));
    EXPECT_LE(sizeof(Result<std::string>), sizeof(std::string) + sizeof(void *));
}
