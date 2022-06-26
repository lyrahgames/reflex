#include <string>
//
#include <SFML/Graphics.hpp>
//
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
using namespace gl;
//
#include <libviewer/viewer.ipp>

using namespace std;

int main(int argc, char* argv[]) {
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
  window.setActive(true);

  glbinding::initialize(sf::Context::getFunction);

  viewer viewer;
  viewer.load_model(argv[1]);

  auto old_mouse_pos = sf::Mouse::getPosition(window);
  bool running = true;
  while (running) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        running = false;
      else if (event.type == sf::Event::Resized)
        viewer.resize(event.size.width, event.size.height);
      else if (event.type == sf::Event::MouseWheelScrolled)
        viewer.zoom(0.1 * event.mouseWheelScroll.delta);
      else if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
          case sf::Keyboard::Escape:
            running = false;
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
    }

    viewer.update();
    viewer.render();

    window.display();
  }
}
