#include <dots/Application.h>
#include <dots/type/AnyStruct.h>
#include <dots/tools/logging.h>
#include <DotsDescriptorRequest.dots.h>
#include <DotsEcho.dots.h>
#include <StructDescriptorData.dots.h>
#include <EnumDescriptorData.dots.h>

int main(int argc, char* argv[])
{
    const std::string AppName = "object-reader";

    // only show errors in log output
    dots::tools::loggingFrontend().setLogLevel(dots::tools::Level::error);

    try
    {
        // parse type name
        if (argc == 1)
        {
            throw std::runtime_error{ "no object argument given" };
        }

        std::string objectString = argv[argc - 1];
        auto typeNamePos = objectString.find_first_of('{');

        if (typeNamePos == std::string::npos)
        {
            throw std::runtime_error{ "could not parse type name of object argument '" + objectString + "'"};
        }

        std::string typeName = objectString.substr(0, typeNamePos);

        // create application
        dots::Application app{ AppName, argc, argv };
        
        // request descriptor for parsed type
        dots::publish(DotsDescriptorRequest{
            DotsDescriptorRequest::whitelist_i{ dots::vector_t<dots::string_t>{ typeName } }
        });

        // asynchronously wait until descriptor has been received
        dots::subscribe<StructDescriptorData>([](auto&){}).discard();
        dots::subscribe<EnumDescriptorData>([](auto&){}).discard();
        dots::subscribe<dots::type::StructDescriptor<>>([&](const dots::type::StructDescriptor<>& descriptor)
        {
            if (descriptor.name() == typeName)
            {
                try
                {
                    // parse and publish object
                    dots::type::AnyStruct instance{ descriptor };
                    dots::from_string(objectString, *instance);
                    dots::publish(instance);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "error reading object argument '" << objectString << "' -> " << e.what() << "\n";
                }

                // publish closing echo
                dots::publish(DotsEcho{
                    DotsEcho::request_i{ true },
                    DotsEcho::data_i{ AppName }
                });
            }
        }).discard();

        // asynchronously wait until object has been read and published
        dots::subscribe<DotsEcho>([&](const dots::Event<DotsEcho>& event)
        {
            if (const DotsEcho& echo = event(); echo.data == AppName)
            {
                app.exit(EXIT_SUCCESS);
            }
        }).discard();

        // execute event loop of application 
        return app.exec();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR running " << AppName << " -> " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "ERROR running " << AppName << " -> <unknown exception>" << "\n";
        return EXIT_FAILURE;
    }
}
