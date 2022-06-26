#include <SFML/Graphics.hpp>
//
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
using namespace gl;
//
#include <libviewer/viewer.ipp>

int main() {
  using viewer::viewer;
  sf::ContextSettings settings;
  settings.majorVersion = viewer::context_version_major;
  settings.minorVersion = viewer::context_version_minor;

  sf::Window window(sf::VideoMode(viewer::initial_screen_width,
                                  viewer::initial_screen_height),
                    viewer::title, sf::Style::Default, settings);
  window.setVerticalSyncEnabled(true);
  window.setActive(true);

  glbinding::initialize(sf::Context::getFunction);

  viewer viewer;

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
    }

    viewer.update();
    viewer.render();

    window.display();
  }
}
