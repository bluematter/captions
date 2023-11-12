#include <iostream>
#include <string>
#include <vector>
#include "captions.h"

struct Arguments
{
    std::string input_source;
    std::string segments_source;
    std::string output_name;
    std::string font;
    bool highlighter;
    std::string text_color;
};

Arguments parse_arguments(int argc, char *argv[])
{
    Arguments args;

    std::cout << "argc: " << argc << std::endl;

    if (argc != 13)
    {
        std::cerr << "Invalid number of arguments. Usage: ./captions --input <input_source> --segments <segments_source> --output <output_name> --font <font> --highlighter <highlighter> --text_color <text_color>" << std::endl;
        return args;
    }

    for (int i = 1; i < argc; i += 2)
    {
        std::string flag = argv[i];
        std::string value = argv[i + 1];

        if (flag == "--input")
        {
            args.input_source = value;
        }
        else if (flag == "--segments")
        {
            args.segments_source = value;
        }
        else if (flag == "--output")
        {
            args.output_name = value;
        }
        else if (flag == "--font")
        {
            args.font = value;
        }
        else if (flag == "--highlighter")
        {
            args.highlighter = (value == "true");
        }
        else if (flag == "--text_color")
        {
            args.text_color = value;
        }
        else
        {
            std::cerr << "Invalid flag: " << flag << std::endl;
            return args;
        }
    }

    return args;
}

int main(int argc, char *argv[])
{
    Arguments args = parse_arguments(argc, argv);

    // Access the parsed arguments
    std::cout << "Input Source: " << args.input_source << std::endl;
    std::cout << "Segments Source: " << args.segments_source << std::endl;
    std::cout << "Output Name: " << args.output_name << std::endl;
    std::cout << "Font: " << args.font << std::endl;
    std::cout << "Highlighter: " << std::boolalpha << args.highlighter << std::endl;
    std::cout << "Text Color: " << args.text_color << std::endl;

    // Pass the arguments to captions.cpp
    captions(args.output_name, args.segments_source, args.text_color, args.highlighter, args.font, args.input_source);

    return 0;
}
