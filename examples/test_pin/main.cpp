// Demonstrates PinPtr: a shared firm reference that can point to a member /
// sub-object owned by a FirmPtr, keeping the whole owner alive as long as the
// pin lives. Uses makeStandalone so no pool bookkeeping is needed - the owner
// self-destructs once its last firm/pin reference drops.

#include "dynasma/pin.hpp"
#include "dynasma/pointer.hpp"
#include "dynasma/standalone.hpp"

#include <iostream>
#include <string>

// A non-polymorphic sub-object, owned as a member by Scene below.
struct Camera {
    std::string name;

    Camera(std::string n) : name(std::move(n)) {
        std::cout << "        Camera '" << name << "' constructed\n";
    }

    ~Camera() { std::cout << "        Camera '" << name << "' destructed\n"; }
};

// The owning asset. Camera is a plain member - its lifetime is bound to Scene.
struct Scene : public dynasma::PolymorphicBase {
    Camera camera;

    Scene(std::string cameraName) : camera(std::move(cameraName)) {
        std::cout << "    Scene constructed\n";
    }

    ~Scene() { std::cout << "    Scene destructed\n"; }

    std::size_t memory_cost() const { return sizeof(Scene); }
};

dynasma::PinPtr<Camera> createSceneAndGetCamera() {

    std::cout << "Creating standalone Scene...\n";

    // Own the Scene through a FirmPtr.
    dynasma::FirmPtr p_scene = dynasma::makeStandalone<Scene>("main-cam");

    // A PinPtr can adopt the whole owner...
    dynasma::PinPtr<Scene> scenePin = p_scene;

    // ...and from that, pin an owned member sub-object. The pin shares the
    // owner's reference count, so the Camera pointer stays valid as long as
    // *any* pin into the Scene lives - even after Scene's FirmPtr is gone.
    dynasma::PinPtr<Camera> p_camera{scenePin, scenePin->camera};

    std::cout << "    Pointed-to camera: " << p_camera->name << "\n";
    std::cout << "    Exiting block... dropping FirmPtr to Scene, keeping "
                 "PinPtr to Camera\n";

    return p_camera;
}

int main() {
    // PinPtr must outlive the FirmPtr to the owner to prove that the
    // sub-object pin keeps the owning Scene alive on its own.
    dynasma::PinPtr<Camera> p_camera = createSceneAndGetCamera();

    // Scene has NOT been destructed here: cameraPin still holds it.
    std::cout << "After block, camera still alive: " << p_camera->name << "\n";

    // Dropping the last pin releases the Scene: expect Camera then Scene
    // destructors to fire right here.
    std::cout << "Dropping last camera PinPtr...\n";

    return 0;
}
