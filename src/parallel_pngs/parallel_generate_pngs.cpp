#include "parallel_generate_pngs.h"
#include <thread>
#include <cairo.h>
#include <pango/pangocairo.h>
#include <glib-object.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cctype>
#include <iostream>

std::string uppercase(const std::string &input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c){ return std::toupper(c); });
    return result;
}

void draw_rounded_rectangle(cairo_t *cr, double x, double y, double width, double height, double corner_radius, double red, double green, double blue)
{
    double degrees = M_PI / 180.0;

    cairo_new_sub_path(cr);
    cairo_arc(cr, x + width - corner_radius, y + corner_radius, corner_radius, -90 * degrees, 0 * degrees);
    cairo_arc(cr, x + width - corner_radius, y + height - corner_radius, corner_radius, 0 * degrees, 90 * degrees);
    cairo_arc(cr, x + corner_radius, y + height - corner_radius, corner_radius, 90 * degrees, 180 * degrees);
    cairo_arc(cr, x + corner_radius, y + corner_radius, corner_radius, 180 * degrees, 270 * degrees);
    cairo_close_path(cr);

    cairo_set_source_rgb(cr, red, green, blue); // set color, change as needed
    cairo_fill(cr);
}

int get_line_width(const std::vector<int> &word_widths, size_t start, size_t end)
{
    int width = 0;
    for (size_t i = start; i < end; ++i)
    {
        width += word_widths[i] + 10; // Add 10 for space between words
    }
    return width;
}

void generate_pngs(const nlohmann::json& j, int scale, int width, int height, std::string font, bool highlighter, const std::string& text_color) {
    width *= scale; // or 1080 for portrait
    height *= scale; // or 1920 for portrait
    int padding = 100 * scale; // adjust as needed
    int line_height = 60 * scale; // Height of a line, adjust as needed
    int rect_height = 50 * scale;

    // Set the text color (red, green, blue)
    int r, g, b;
    std::sscanf(text_color.c_str(), "#%02x%02x%02x", &r, &g, &b);
    double red = r / 255.0;
    double green = g / 255.0;
    double blue = b / 255.0;

    std::string hex_color = "#2d9488";
    int bg_r, bg_g, bg_b;
    std::sscanf(hex_color.c_str(), "#%02x%02x%02x", &r, &g, &b);

    double bg_red = bg_r / 255.0;
    double bg_green = bg_g / 255.0;
    double bg_blue = bg_b / 255.0;

    double vertical_position_factor = 0.8;

    // Iterate over each segment in the json
    for (const auto& segment : j) {
        std::vector<int> word_widths;
        std::vector<int> breakpoints;

        // Check if the segment contains "words"
        if (segment.find("words") != segment.end()) {
            // Extract the words
            const auto& words = segment["words"];

            // Calculate the width of each word and the total width
            for (size_t i = 0; i < words.size(); ++i) {
                std::string word = words[i]["word"];
                word.erase(0, word.find_first_not_of(' '));

                // Convert the word to uppercase
                word = uppercase(word);

                cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
                cairo_t *cr = cairo_create(surface);

                PangoLayout *layout = pango_cairo_create_layout(cr);
                pango_layout_set_text(layout, word.c_str(), -1);
                PangoFontDescription *desc = pango_font_description_from_string(font.c_str());
                pango_layout_set_font_description(layout, desc);

                int word_width, word_height;
                pango_layout_get_pixel_size(layout, &word_width, &word_height);
                word_widths.push_back(word_width);

                pango_font_description_free(desc);
                g_object_unref(layout);
                cairo_destroy(cr);
                cairo_surface_destroy(surface);
            }

            // Calculate the breakpoints for the text
            breakpoints = {0};
            int current_width = 0;
            for (size_t i = 0; i < word_widths.size(); ++i) {
                // Include the width of the space
                int total_word_width = word_widths[i] + 10;
                if (current_width + total_word_width > width - 2 * padding) {
                    breakpoints.push_back(i);
                    current_width = total_word_width;
                } else {
                    current_width += total_word_width;
                }
            }
            breakpoints.push_back(word_widths.size());

            // Loop over each spoken word
            for (size_t i = 0; i < words.size(); ++i) {
                // Create the PNG image here
                cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
                cairo_t *cr = cairo_create(surface);

                PangoLayout *layout;
                PangoFontDescription *desc;
                layout = pango_cairo_create_layout(cr);

                int total_text_height = (breakpoints.size() - 1) * line_height;

                // Loop over each line
                for (size_t b = 0; b < breakpoints.size() - 1; ++b) {
                    // int line_width = get_line_width(word_widths, breakpoints[b], breakpoints[b + 1]);
                    // int current_x_position = (width - line_width) / 2; // Center the line
                    // int current_y_position = vertical_position_factor * (height - total_text_height) + b * line_height;

                    // Loop over each word in the current line
                    for (size_t j = breakpoints[b]; j < breakpoints[b + 1]; ++j) {
                        if (i != j) continue;
                        std::string current_word = words[j]["word"];
                        current_word.erase(0, current_word.find_first_not_of(' '));

                        // Convert the word to uppercase
                        current_word = uppercase(current_word);

                        int single_word_width, single_word_height;
                        pango_layout_set_text(layout, current_word.c_str(), -1);
                        desc = pango_font_description_from_string(font.c_str());
                        pango_layout_set_font_description(layout, desc);
                        pango_layout_get_pixel_size(layout, &single_word_width, &single_word_height);
                        int centered_x_position = (width - single_word_width) / 2;
                        int centered_y_position = (height - single_word_height) / 2;

                        // hack that draws a rectangle around the word to refresh rendering
                        cairo_set_line_width(cr, 0); // Adjust the width as needed
                        cairo_set_source_rgb(cr, 1, 0, 0); // RGB for red
                        cairo_rectangle(cr, centered_x_position, centered_y_position, single_word_width, single_word_height);

                        // Stroke the rectangle to actually draw it
                        cairo_stroke(cr);

                         if (segment["id"].get<int>() == 0) {
                            // std::cout << "current_word " << current_word << std::endl;
                            // std::cout << "spoken_word " << (i == j) << std::endl;
                            // std::cout << "current_word_start " << words[j]["start"] << std::endl;
                            // std::cout << "current_word_end " << words[j]["end"] << std::endl;
                            // std::cout << "segment " << segment["id"].get<int>() << std::endl;
                            std::cout << "centered_x_position " << centered_x_position << std::endl;
                            std::cout << "width " << width << std::endl;
                            std::cout << "single_word_width " << single_word_width << std::endl;
                        }

                        // If the word is the spoken word, draw the background and set the color to RGB
                        if (i == j && highlighter) {
                            // int text_height;
                            // pango_layout_get_pixel_size(layout, NULL, &text_height);
                            // int rect_width = word_widths[j] + 10; // or any other calculation for the width
                            // int centered_y_position = current_y_position + (line_height - text_height) / 2 + 2;
                            // int centered_x_position = current_x_position + word_widths[j] / 2 - rect_width / 2;

                            // // Ensure that the rectangle always stays on screen
                            // centered_x_position = std::max(centered_x_position, 0);
                            // centered_y_position = std::max(centered_y_position, 0);

                            // draw_rounded_rectangle(cr, centered_x_position, centered_y_position, rect_width, rect_height, 5, bg_red, bg_green, bg_blue);
                            // cairo_set_source_rgb(cr, bg_red, bg_green, bg_blue);
                            // cairo_fill(cr);

                            // // Draw the text shadow
                            // cairo_move_to(cr, current_x_position + 2, current_y_position + 2); // Offset for shadow
                            // cairo_set_source_rgb(cr, 0, 0, 0); // Shadow color
                            // pango_cairo_show_layout(cr, layout);

                            // // Draw the actual text
                            // cairo_move_to(cr, current_x_position, current_y_position);
                            // cairo_set_source_rgb(cr, red, green, blue); // Text color
                            // pango_cairo_show_layout(cr, layout);
                        } else {
                            // Draw the text shadow
                            cairo_move_to(cr, centered_x_position + 4, centered_y_position + 4); // Offset for shadow
                            cairo_set_source_rgb(cr, 0, 0, 0); // Shadow color
                            pango_cairo_show_layout(cr, layout);

                            // Draw the actual text
                            cairo_move_to(cr, centered_x_position, centered_y_position);
                            cairo_set_source_rgb(cr, red, green, blue); // Text color
                            pango_cairo_show_layout(cr, layout);
                        }

                        // Update the position and show the layout
                        cairo_move_to(cr, centered_x_position, centered_y_position);
                        pango_cairo_show_layout(cr, layout);

                        // current_x_position += word_widths[j] + 10; // Added 10 for space between words
                    }

                    // Reset the x position and move down the y position for the next line
                    // current_x_position = padding;
                    // current_y_position += line_height;
                }

                // Write to a different PNG file for each spoken word
                std::string filename = "/tmp/captions/frame" + std::to_string(segment["id"].get<int>()) + "_" + std::to_string(i) + ".png";
                cairo_surface_write_to_png(surface, filename.c_str());

                // Clean up
                pango_font_description_free(desc);
                g_object_unref(layout);
                cairo_destroy(cr);
                cairo_surface_destroy(surface);
            }
        
        }
    }
}

void parallel_generate_pngs(nlohmann::json& segments, int scale, int width, int height, std::string font, unsigned int num_threads, bool highlighter, const std::string& text_color) {
    // Calculate the size of each chunk
    size_t chunk_size = segments.size() / num_threads;

    // Create a vector to hold the threads
    std::vector<std::thread> threads;

    // Create and launch the threads
    for (unsigned int i = 0; i < num_threads; ++i) {
        // Calculate the start and end of this chunk
        auto start = segments.begin() + i * chunk_size;
        auto end = (i == num_threads - 1) ? segments.end() : start + chunk_size;

        // Create a thread to handle this chunk
        threads.push_back(std::thread(generate_pngs, nlohmann::json(start, end), scale, width, height, font, highlighter, text_color));
    }

    // Join the threads (wait for them to finish)
    for (auto& thread : threads) {
        thread.join();
    }
}
