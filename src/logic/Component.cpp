#include "../includes/Component.h"

uint32_t Component::next_uid = 0;
std::vector<Component*> Component::components;

void Component::evaluateComponent() {}

void Component::evaluateSystem() {
    for (Component* comp : components) {
        comp->evaluateComponent();
#ifdef DEBUG
        std::cout << "Component: "<< comp->getName() << " UID: " << comp->getUid() << ", Type: " << comp->typeToString()  
                  << ", Output State: " << (comp->output->getState() == WIRE_STATE::LOGIC_HIGH ? "HIGH" : "LOW") << std::endl;
#endif
    }
}