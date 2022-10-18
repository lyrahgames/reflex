#include <string>
//
#include <SFML/Graphics.hpp>
//
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
using namespace gl;
//
#include <libviewer/viewer.ipp>
//
#include <lyrahgames/options/options.hpp>
//
#include <lyrahgames/log/log.hpp>

using namespace std;
// Provide the list of program options and
// the variable which is able to store their values.
using namespace lyrahgames::options;
option_list<  //
    flag<{"help", 'h'}, "Print the help message.">,
    flag<"version", "Print the library version.">,
    flag<{"daemon", 'd'}, "Run application as daemon.">,
    attachment<{"shader", 's'}, "Path to shader">,
    attachment<{"model", 'm'}, "Path to model">>
    options{};
using positioning = position_list<"model">;

int main(int argc, char* argv[]) {
  lyrahgames::log::log log;

  try {
    parse<positioning>({argc, argv}, options);
  } catch (parser_error& e) {
    log.error(e.what());
    // cerr << e.what() << endl;
    exit(-1);
  }

  // Provide a custom help message.
  if (option<"help">(options)) {
    for_each(options, [](auto& option) {
      cout << left << setw(25) << option.help() << option.description() << endl;
    });
    exit(0);
  }

  // Provide the library version.
  // if (option<"version">(options)) {
  //   cout << "Version " LYRAHGAMES_OPTIONS_VERSION_STR << endl;
  //   exit(0);
  // }

  if (option<"daemon">(options)) daemon(true, true);

  using viewer::viewer;
  sf::ContextSettings settings;
  settings.majorVersion = viewer::context_version_major;
  settings.minorVersion = viewer::context_version_minor;
  // These values need to be set when 3D rendering is required.
  settings.depthBits = 24;
  settings.stencilBits = 8;
  settings.antialiasingLevel = 4;

  sf::Window window(sf::VideoMode(viewer::initial_screen_width,
                                  viewer::initial_screen_height),
                    (string("SFML ") + viewer::title).c_str(),
                    sf::Style::Default, settings);
  window.setVerticalSyncEnabled(true);
  window.setKeyRepeatEnabled(false);
  window.setActive(true);

  glbinding::initialize(sf::Context::getFunction);

  viewer viewer;

  if (option<"shader">(options)) viewer.load_shader(value<"shader">(options));
  if (option<"model">(options)) viewer.load_model(value<"model">(options));

  viewer.start();

  bool drawing = false;

  auto old_mouse_pos = sf::Mouse::getPosition(window);
  while (viewer.running()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        viewer.stop();
      else if (event.type == sf::Event::Resized)
        viewer.resize(event.size.width, event.size.height);
      else if (event.type == sf::Event::MouseWheelScrolled)
        viewer.zoom(0.1 * event.mouseWheelScroll.delta);
      else if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
          case sf::Keyboard::Escape:
            viewer.stop();
            break;
          case sf::Keyboard::P:
            viewer.preprocess_curve();
            break;
          case sf::Keyboard::Space:
            // viewer.select_vertex(old_mouse_pos.x, old_mouse_pos.y);
            drawing = !drawing;
            break;
        }
      }
    }

    // Get new mouse position and compute movement in space.
    const auto mouse_pos = sf::Mouse::getPosition(window);
    const auto mouse_move = mouse_pos - old_mouse_pos;
    old_mouse_pos = mouse_pos;

    if (window.hasFocus()) {
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        viewer.turn({0.01 * mouse_move.x, 0.01 * mouse_move.y});
      if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
        viewer.shift({mouse_move.x, mouse_move.y});

      // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
      if (drawing && (mouse_move != sf::Vector2i{}))
        viewer.select_vertex(old_mouse_pos.x, old_mouse_pos.y);
    }

    viewer.update();
    viewer.render();

    window.display();
  }
}
