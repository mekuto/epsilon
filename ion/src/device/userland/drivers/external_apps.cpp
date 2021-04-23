#include <ion/external_apps.h>
#include <assert.h>

extern "C" {
  uint8_t _external_apps_start;
}

namespace Ion {
namespace ExternalApps {

/* The ExternalApp start with its info layout as:
 * - 4 bytes: a magic code 0XBABECODE
 * - 4 bytes: the API level of the AppInfo layout
 * - 4 bytes: the address of the app name
 * - 4 bytes: the address of the upper app name
 * - 4 bytes: the address of the compressed icon data
 * - 4 bytes: the address of the entry point
 * - 4 bytes: the size of the external app including the AppInfo header
 * - 4 bytes: the same magic code 0xBABECODE
 */

App::App(uint8_t * a) : m_startAddress(a) {
  assert(*reinterpret_cast<uint32_t *>(m_startAddress) == k_magic);
}

const char * App::name() const {
  return *reinterpret_cast<const char **>(m_startAddress + 2*sizeof(uint32_t));
}

const char * App::upperName() const {
  return *reinterpret_cast<const char **>(m_startAddress + 3*sizeof(uint32_t));
}

const uint8_t * App::iconData() const {
  return *reinterpret_cast<const uint8_t **>(m_startAddress + 5*sizeof(uint32_t));
}

void * App::entryPoint() const {
  return *reinterpret_cast<void **>(m_startAddress + 5*sizeof(uint32_t));
}

bool App::appAtAddress(uint8_t * address) {
  return *reinterpret_cast<uint32_t *>(address) == k_magic;
}

AppIterator & AppIterator::operator++() {
  uint32_t sizeOfCurrentApp = *reinterpret_cast<uint32_t *>(m_currentAddress + 6*sizeof(uint32_t));
  m_currentAddress += sizeOfCurrentApp;
  if (!App::appAtAddress(m_currentAddress)) {
    m_currentAddress = nullptr;
  }
  return *this;
}

AppIterator Apps::begin() const {
  if (!App::appAtAddress(&_external_apps_start)) {
    return end();
  }
  return AppIterator(&_external_apps_start);
}

int numberOfApps() {
  int counter = 0;
  for (App a : Apps()) {
    counter++;
  }
  return counter;
}

}
}
