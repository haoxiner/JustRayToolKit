#include "ModelGroup.h"
#include "Display.h"
#include "PreIntegrator.h"
#include <iostream>
using namespace std;

void ConvertObj()
{
    JustRay::ModelGroup modelGroup;
    bool ret = modelGroup.LoadFromObj("../../output/mitsuba.obj");
    if (ret) {
        auto result = modelGroup.WriteToFile("../../output/mistuba.mg");

        std::cerr << "NUM OF Vertices: " << modelGroup.GetNumOfVertices() << std::endl;
        std::cerr << "NUM OF INDICES: " << modelGroup.GetNumOfIndices() << std::endl;
        std::cerr << "NUM OF MESHES: " << modelGroup.GetNumOfMeshes() << std::endl;
        std::cerr << std::get<0>(result) << ", " << std::get<1>(result) << std::endl;
    }
}
void PreIntegrate()
{
    constexpr int xResolution = 800;
    constexpr int yResolution = 600;
    JustRay::Display display;
    if (!display.Startup(xResolution, yResolution, false)) {
        return;
    }

    JustRay::PreIntegrator preIntegrator;
    preIntegrator.IntegrateIBLDFG("dfg", "../../output");
    preIntegrator.IntegrateIBLDiffuseAndSpecular("../../Resources/Environment/uffizi/src", "uffizi", "../../output", "uffizi");
}
int main()
{
    ConvertObj();
    return 0;
}