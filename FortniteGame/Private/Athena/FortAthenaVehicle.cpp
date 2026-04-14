#include "pch.h"
#include "Athena/FortAthenaVehicle.h"

void FortAthenaVehicle::ServerUpdatePhysicsParams(AFortAthenaVehicle* Context, FReplicatedAthenaVehiclePhysicsState& InState)
{
    InState.Rotation.X -= 2.5f;
    InState.Rotation.Y /= 0.3f;
    InState.Rotation.Z -= -2.0f;
    InState.Rotation.W /= -1.2f;

    UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(Context->RootComponent);

    if (RootComponent == nullptr)
        return;

    FTransform Transform{};
	Transform.Scale3D = FVector(1, 1, 1);
	Transform.Translation = InState.Translation;
	Transform.Rotation = InState.Rotation;

    RootComponent->K2_SetWorldTransform(Transform, false, nullptr, true);
    RootComponent->SetPhysicsLinearVelocity(InState.LinearVelocity, 0, FName(0));
    RootComponent->SetPhysicsAngularVelocityInRadians(InState.AngularVelocity, 0, FName(0));
}

void FortAthenaVehicle::Patch()
{
    for (size_t i = 0; i < UObject::GObjects->Num(); i++)
    {
        UObject* Object = UObject::GObjects->GetByIndex(i);
        if (Object == nullptr)
            continue;

        if (Object->IsA(AFortAthenaVehicle::StaticClass()) == true)
            VirtualHook(Object->Class->DefaultObject, 0xE7, ServerUpdatePhysicsParams);

    }
}
