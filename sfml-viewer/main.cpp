#include <SFML/Graphics.hpp>
//
#include <glbinding/glbinding.h>
//
#include <libviewer/viewer.hpp>

int main() {
  sf::ContextSettings settings;
  settings.depthBits = 24;
  settings.stencilBits = 8;
  settings.antialiasingLevel = 4;
  settings.majorVersion = 3;
  settings.minorVersion = 0;

  sf::Window window(sf::VideoMode(800, 450), "OpenGL Test", sf::Style::Default,
                    settings);
  window.setVerticalSyncEnabled(true);
  window.setActive(true);

  glbinding::initialize(sf::Context::getFunction);

  viewer::viewer viewer(800, 450);

  bool running = true;
  while (running) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        running = false;
      else if (event.type == sf::Event::Resized)
        viewer.resize(event.size.width, event.size.height);
    }

    viewer.update();
    viewer.render();

    window.display();
  }
}
