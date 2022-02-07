#include <iostream>
#include <dots/Application.h>
#include <dots/tools/logging.h>
#include <DotsDescriptorRequest.dots.h>
#include <StructDescriptorData.dots.h>
#include <EnumDescriptorData.dots.h>

int main(int argc, char* argv[])
{
    const std::string AppName = "object-trace";

    // only show errors in log output
    dots::tools::loggingFrontend().setLogLevel(dots::tools::Level::error);

    try
    {
        // create application
        dots::Application app{ AppName, argc, argv };

        // request descriptors for all currently known types
        dots::publish(DotsDescriptorRequest{
            DotsDescriptorRequest::whitelist_i{}
        });

        // subscribe to all DOTS struct type descriptors
        dots::subscribe<StructDescriptorData>([](auto&){}).discard();
        dots::subscribe<EnumDescriptorData>([](auto&){}).discard();
        dots::subscribe<dots::type::StructDescriptor<>>([](const dots::type::StructDescriptor<>& descriptor)
        {
            // subscribe to non-internal top-level DOTS struct types
            if (!descriptor.internal() && !descriptor.substructOnly())
            {
                dots::subscribe(descriptor, [](const dots::Event<>& event)
                {
                    // print instance of DOTS struct type with meta information
                    const DotsHeader& header = event.header();
                    const dots::type::Struct& instance = event();
                    DotsMt lastOperation = event.mt();

                    std::cout << "[" << header.sentTime->toString() << "] ";
                    std::cout << dots::to_string(lastOperation) << ": ";
                    std::cout << dots::to_string(instance) << "\n";
                }).discard();
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
