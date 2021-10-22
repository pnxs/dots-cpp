#include <dots/testing/gtest/gtest.h>
#include <dots/tools/Uri.h>

struct TestUri : ::testing::Test
{
protected:
};

TEST_F(TestUri, ctor_MinimalUriString)
{
    dots::tools::Uri sut{ "tcp://127.0.0.1" };

    EXPECT_EQ(std::string{ sut.scheme() }, "tcp");
    EXPECT_EQ(std::string{ sut.authority() }, "127.0.0.1");
    EXPECT_EQ(std::string{ sut.userName() }, "");
    EXPECT_EQ(std::string{ sut.userPassword() }, "");
    EXPECT_EQ(std::string{ sut.host() }, "127.0.0.1");
    EXPECT_EQ(std::string{ sut.port() }, "");
    EXPECT_EQ(std::string{ sut.path() }, "");
}

TEST_F(TestUri, ctor_CompleteUriString)
{
    dots::tools::Uri sut{ "dots://user:password@foo.bar:42/" };

    EXPECT_EQ(std::string{ sut.scheme() }, "dots");
    EXPECT_EQ(std::string{ sut.authority() }, "user:password@foo.bar:42");
    EXPECT_EQ(std::string{ sut.userName() }, "user");
    EXPECT_EQ(std::string{ sut.userPassword() }, "password");
    EXPECT_EQ(std::string{ sut.host() }, "foo.bar");
    EXPECT_EQ(std::string{ sut.port() }, "42");
    EXPECT_EQ(std::string{ sut.path() }, "/");
}

TEST_F(TestUri, ctor_PartialUriString)
{
    dots::tools::Uri sut{ "ws://user@localhost:12345" };

    EXPECT_EQ(std::string{ sut.scheme() }, "ws");
    EXPECT_EQ(std::string{ sut.authority() }, "user@localhost:12345");
    EXPECT_EQ(std::string{ sut.userName() }, "user");
    EXPECT_EQ(std::string{ sut.userPassword() }, "");
    EXPECT_EQ(std::string{ sut.host() }, "localhost");
    EXPECT_EQ(std::string{ sut.port() }, "12345");
    EXPECT_EQ(std::string{ sut.path() }, "");
}

TEST_F(TestUri, ctor_NoAuthorityUriString)
{
    {
        dots::tools::Uri sut{ "uds:/tmp/dots_uds.socket" };

        EXPECT_EQ(std::string{ sut.scheme() }, "uds");
        EXPECT_EQ(std::string{ sut.authority() }, "");
        EXPECT_EQ(std::string{ sut.userName() }, "");
        EXPECT_EQ(std::string{ sut.userPassword() }, "");
        EXPECT_EQ(std::string{ sut.host() }, "");
        EXPECT_EQ(std::string{ sut.port() }, "");
        EXPECT_EQ(std::string{ sut.path() }, "/tmp/dots_uds.socket");
    }

    {
        dots::tools::Uri sut{ "uds:///tmp/dots_uds.socket" };

        EXPECT_EQ(std::string{ sut.scheme() }, "uds");
        EXPECT_EQ(std::string{ sut.authority() }, "");
        EXPECT_EQ(std::string{ sut.userName() }, "");
        EXPECT_EQ(std::string{ sut.userPassword() }, "");
        EXPECT_EQ(std::string{ sut.host() }, "");
        EXPECT_EQ(std::string{ sut.port() }, "");
        EXPECT_EQ(std::string{ sut.path() }, "/tmp/dots_uds.socket");
    }
}

TEST_F(TestUri, ctor_Ipv6UriString)
{
    {
        dots::tools::Uri sut{ "tcp://user@[1080:0:0:0:8:800:200C:417A]:12345/" };

        EXPECT_EQ(std::string{ sut.scheme() }, "tcp");
        EXPECT_EQ(std::string{ sut.authority() }, "user@[1080:0:0:0:8:800:200C:417A]:12345");
        EXPECT_EQ(std::string{ sut.userName() }, "user");
        EXPECT_EQ(std::string{ sut.userPassword() }, "");
        EXPECT_EQ(std::string{ sut.host() }, "1080:0:0:0:8:800:200C:417A");
        EXPECT_EQ(std::string{ sut.port() }, "12345");
        EXPECT_EQ(std::string{ sut.path() }, "/");
    }

    {
        dots::tools::Uri sut{ "ws://user:password@[::FFFF:129.144.52.38]" };

        EXPECT_EQ(std::string{ sut.scheme() }, "ws");
        EXPECT_EQ(std::string{ sut.authority() }, "user:password@[::FFFF:129.144.52.38]");
        EXPECT_EQ(std::string{ sut.userName() }, "user");
        EXPECT_EQ(std::string{ sut.userPassword() }, "password");
        EXPECT_EQ(std::string{ sut.host() }, "::FFFF:129.144.52.38");
        EXPECT_EQ(std::string{ sut.port() }, "");
        EXPECT_EQ(std::string{ sut.path() }, "");
    }
}

TEST_F(TestUri, setScheme)
{
    {
        dots::tools::Uri sut{ "tcp://127.0.0.1" };
        EXPECT_THROW(sut.setScheme(""), std::runtime_error);
    }

    {
        dots::tools::Uri sut{ "tcp://127.0.0.1" };
        sut.setScheme("ws");

        EXPECT_EQ(std::string{ sut.scheme() }, "ws");
        EXPECT_EQ(std::string{ sut.authority() }, "127.0.0.1");
        EXPECT_EQ(std::string{ sut.path() }, "");
    }

    {
        dots::tools::Uri sut{ "dots://user:password@foo.bar:42/" };
        sut.setScheme("tcp");

        EXPECT_EQ(std::string{ sut.scheme() }, "tcp");
        EXPECT_EQ(std::string{ sut.authority() }, "user:password@foo.bar:42");
        EXPECT_EQ(std::string{ sut.path() }, "/");
    }
}

TEST_F(TestUri, setAuthority)
{
    {
        dots::tools::Uri sut{ "tcp://127.0.0.1" };
        sut.setAuthority("user@localhost:12345");

        EXPECT_EQ(std::string{ sut.scheme() }, "tcp");
        EXPECT_EQ(std::string{ sut.authority() }, "user@localhost:12345");
        EXPECT_EQ(std::string{ sut.path() }, "");
    }

    {
        dots::tools::Uri sut{ "dots://user:password@foo.bar:42/" };
        sut.setAuthority("user@[1080:0:0:0:8:800:200C:417A]:12345");

        EXPECT_EQ(std::string{ sut.scheme() }, "dots");
        EXPECT_EQ(std::string{ sut.authority() }, "user@[1080:0:0:0:8:800:200C:417A]:12345");
        EXPECT_EQ(std::string{ sut.path() }, "/");
    }
}

TEST_F(TestUri, setUserName)
{
    {
        dots::tools::Uri sut{ "tcp://127.0.0.1" };
        sut.setUserName("user");

        EXPECT_EQ(std::string{ sut.scheme() }, "tcp");
        EXPECT_EQ(std::string{ sut.authority() }, "user@127.0.0.1");
        EXPECT_EQ(std::string{ sut.path() }, "");
    }

    {
        dots::tools::Uri sut{ "dots://user:password@foo.bar:42/" };
        sut.setUserName("foobar");

        EXPECT_EQ(std::string{ sut.scheme() }, "dots");
        EXPECT_EQ(std::string{ sut.authority() }, "foobar:password@foo.bar:42");
        EXPECT_EQ(std::string{ sut.path() }, "/");
    }
}

TEST_F(TestUri, setUserPassword)
{
    {
        dots::tools::Uri sut{ "tcp://127.0.0.1" };
        EXPECT_THROW(sut.setUserPassword("password"), std::runtime_error);
    }

    {
        dots::tools::Uri sut{ "ws://user@localhost:12345" };
        sut.setUserPassword("password");

        EXPECT_EQ(std::string{ sut.scheme() }, "ws");
        EXPECT_EQ(std::string{ sut.authority() }, "user:password@localhost:12345");
        EXPECT_EQ(std::string{ sut.path() }, "");
    }

    {
        dots::tools::Uri sut{ "dots://user:password@foo.bar:42/" };
        sut.setUserPassword("");

        EXPECT_EQ(std::string{ sut.scheme() }, "dots");
        EXPECT_EQ(std::string{ sut.authority() }, "user@foo.bar:42");
        EXPECT_EQ(std::string{ sut.path() }, "/");
    }
}

TEST_F(TestUri, setHost)
{
    {
        dots::tools::Uri sut{ "tcp://127.0.0.1" };
        sut.setHost("localhost");

        EXPECT_EQ(std::string{ sut.scheme() }, "tcp");
        EXPECT_EQ(std::string{ sut.authority() }, "localhost");
        EXPECT_EQ(std::string{ sut.path() }, "");
    }

    {
        dots::tools::Uri sut{ "dots://user:password@foo.bar:42/" };
        sut.setHost("1080:0:0:0:8:800:200C:417A");

        EXPECT_EQ(std::string{ sut.scheme() }, "dots");
        EXPECT_EQ(std::string{ sut.authority() }, "user:password@[1080:0:0:0:8:800:200C:417A]:42");
        EXPECT_EQ(std::string{ sut.path() }, "/");
    }

    {
        dots::tools::Uri sut{ "tcp://user@[1080:0:0:0:8:800:200C:417A]:12345/" };
        sut.setHost("127.0.0.1");

        EXPECT_EQ(std::string{ sut.scheme() }, "tcp");
        EXPECT_EQ(std::string{ sut.authority() }, "user@127.0.0.1:12345");
        EXPECT_EQ(std::string{ sut.path() }, "/");
    }
}

TEST_F(TestUri, setPort)
{
    {
        dots::tools::Uri sut{ "tcp://127.0.0.1" };
        sut.setPort("12345");

        EXPECT_EQ(std::string{ sut.scheme() }, "tcp");
        EXPECT_EQ(std::string{ sut.authority() }, "127.0.0.1:12345");
        EXPECT_EQ(std::string{ sut.path() }, "");
    }

    {
        dots::tools::Uri sut{ "dots://user:password@foo.bar:42/" };
        sut.setPort("12345");

        EXPECT_EQ(std::string{ sut.scheme() }, "dots");
        EXPECT_EQ(std::string{ sut.authority() }, "user:password@foo.bar:12345");
        EXPECT_EQ(std::string{ sut.path() }, "/");
    }

    {
        dots::tools::Uri sut{ "dots://user:password@foo.bar:42/" };
        sut.setPort("");

        EXPECT_EQ(std::string{ sut.scheme() }, "dots");
        EXPECT_EQ(std::string{ sut.authority() }, "user:password@foo.bar");
        EXPECT_EQ(std::string{ sut.path() }, "/");
    }
}

TEST_F(TestUri, setPath)
{
    {
        dots::tools::Uri sut{ "tcp://127.0.0.1" };
        EXPECT_THROW(sut.setUserPassword("dots"), std::runtime_error);
        sut.setPath("/dots");

        EXPECT_EQ(std::string{ sut.scheme() }, "tcp");
        EXPECT_EQ(std::string{ sut.authority() }, "127.0.0.1");
        EXPECT_EQ(std::string{ sut.path() }, "/dots");
    }

    {
        dots::tools::Uri sut{ "dots://user:password@foo.bar:42/" };
        sut.setPath("");

        EXPECT_EQ(std::string{ sut.scheme() }, "dots");
        EXPECT_EQ(std::string{ sut.authority() }, "user:password@foo.bar:42");
        EXPECT_EQ(std::string{ sut.path() }, "");
    }
}