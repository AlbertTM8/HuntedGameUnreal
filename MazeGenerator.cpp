// Fill out your copyright notice in the Description page of Project Settings.

#include "MazeGenerator.h"
#include "Math/Vector.h"


// Sets default values
AMazeGenerator::AMazeGenerator()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMazeGenerator::BeginPlay()
{

    Super::BeginPlay();
    Maze.Init();
    Step(StartX, StartY);
    MakeRooms();
    Draw();
    PlaceRoomWalls();
    FString MazeString = "";
    for(int i = 0; i<Width*Height; i++){
        MazeString.AppendInt(Maze.GetValue(i%Width, i/Width));
    }
    UE_LOG(LogTemp, Warning, TEXT("%s"), *MazeString);
}

void AMazeGenerator::Step(int32 X, int32 Y)
{
    Directions.Shuffle();

    for (int32 i = 0; i < Directions.Num(); i++) {
        switch (Directions[i])
        {
        case 0: // Up
            if (X + 2 >= Width - 1 || Maze.GetValue(X + 2, Y) == 0)
                continue;

            Maze.SetValue(X + 2, Y, 0);
            Maze.SetValue(X + 1, Y, 0);
            Step(X + 2, Y);

        case 1: // Right
            if (Y + 2 >= Height - 1 || Maze.GetValue(X, Y + 2) == 0)
                continue;

            Maze.SetValue(X, Y + 2, 0);
            Maze.SetValue(X, Y + 1, 0);
            Step(X, Y + 2);

        case 2: // Down
            if (X - 2 <= 0 || Maze.GetValue(X - 2, Y) == 0)
                continue;

            Maze.SetValue(X - 2, Y, 0);
            Maze.SetValue(X - 1, Y, 0);
            Step(X - 2, Y);

        case 3: // Left
            if (Y - 2 <= 0 || Maze.GetValue(X, Y - 2) == 0)
                continue;

            Maze.SetValue(X, Y - 2, 0);
            Maze.SetValue(X, Y - 1, 0);
            Step(X, Y - 2);
        }
    }
}


void AMazeGenerator::Draw() const
{
    for (int32 x = 1; x < Width - 1; x++)
    {
        for (int32 y = 1; y < Height - 1; y++)
        {
            // Straight
            if      (IsPatternMatching(x, y, HorizontalStraightPattern)) { PlacePiece(x, y, 90.f, StraightPiece); }
            else if (IsPatternMatching(x, y, VerticalStraightPattern))   { PlacePiece(x, y,  0.f, StraightPiece); }

            // Turns
            else if (IsPatternMatching(x, y, TurnLeftUpPattern))    { PlacePiece(x, y, 0.f, TurnPiece);   }
            else if (IsPatternMatching(x, y, TurnLeftDownPattern))  { PlacePiece(x, y, 90.f, TurnPiece);  }
            else if (IsPatternMatching(x, y, TurnRightUpPattern))   { PlacePiece(x, y, -90.f, TurnPiece); }
            else if (IsPatternMatching(x, y, TurnRightDownPattern)) { PlacePiece(x, y, 180.f, TurnPiece); }

            // T Junctions	
            else if (IsPatternMatching(x, y, TJunctionUpPattern))    { PlacePiece(x, y, -90.f, TJunctionPiece); }
            else if (IsPatternMatching(x, y, TJunctionDownPattern))  { PlacePiece(x, y,  90.f, TJunctionPiece); }
            else if (IsPatternMatching(x, y, TJunctionLeftPattern))  { PlacePiece(x, y,   0.f, TJunctionPiece); }
            else if (IsPatternMatching(x, y, TJunctionRightPattern)) { PlacePiece(x, y, 180.f, TJunctionPiece); }

            // Dead ends
            else if (IsPatternMatching(x, y, DeadEndUpPattern))    { PlacePiece(x, y,  90.f, DeadEndPiece); }
            else if (IsPatternMatching(x, y, DeadEndDownPattern))  { PlacePiece(x, y, -90.f, DeadEndPiece); }
            else if (IsPatternMatching(x, y, DeadEndLeftPattern))  { PlacePiece(x, y, 180.f, DeadEndPiece); }
            else if (IsPatternMatching(x, y, DeadEndRightPattern)) { PlacePiece(x, y,   0.f, DeadEndPiece); }

            // Crossroad
            else if (IsPatternMatching(x, y, CrossroadPattern)) { PlacePiece(x, y, 0.f, CrossroadPiece); }
        }
    }
    for (int i = 0; i < RoomSizes.Num(); i++){
        if(RoomStartPositions.Num() == RoomSizes.Num()*2){
        PlacePiece(RoomStartPositions[i*2]+(RoomSizes[i]/2), RoomStartPositions[(i*2)+1]+(RoomSizes[i]/2), 0.f, RoomPiece);

        }
        else{
            UE_LOG(LogTemp, Warning, TEXT("RoomStartPositions array is not the correct size"));
        }
    }
}


bool AMazeGenerator::IsPatternMatching(int32 X, int32 Y, TArray<int8> Pattern) const
{
    int Count = 0;
    int i = 0;
    bool RoomPart = false;
    for (int y = 1; y > -2; y--)
    {
        for (int x = -1; x < 2; x++)
        {
            if (Pattern[i] == Maze.GetValue(X + x, Y + y) || Pattern[i] == 5)
            {
                Count++;
            }
            if(Maze.GetValue(X + x, Y + y) == 2){
                RoomPart = true;
            }
            i++;
        }
    }

    return Count == 9 && !RoomPart;
}

void AMazeGenerator::PlacePiece(int32 X, int32 Y, float Yaw, TSubclassOf<AActor> Piece) const
{
    FVector Location(X * Scale, Y * Scale, 0);
    FRotator Rotation(0, Yaw, 0);
    FActorSpawnParameters SpawnInfo;

    GetWorld()->SpawnActor<AActor>(Piece, Location, Rotation, SpawnInfo);
}

//Draws rooms in the maze class
void AMazeGenerator::MakeRooms(){
    for (int i = 0; i < RoomSizes.Num(); i++){
        int32 size = RoomSizes[i];
        if(RoomStartPositions[i*2] % 2 != 0){
            RoomStartPositions[i*2]--;
        }
        if(RoomStartPositions[i*2+1] % 2 != 0){
            RoomStartPositions[i*2+1]--;
        }
        int32 start_x = RoomStartPositions[i*2];
        int32 start_y = RoomStartPositions[i*2+1];
        for(int j = start_x; j < start_x+size; j++){
            for(int k = start_y; k < start_y+size; k++){
                Maze.SetValue(j, k, 2);
            }
        }
    }
}
//check if the current position has at least 1 2 in the 3x3 grid and less then 3 2's
bool AMazeGenerator::IsWall(int32 X, int32 Y) const
{
    int Count = 0;
    for (int y = 1; y > -2; y--)
    {
        for (int x = -1; x < 2; x++)
        {
            if (Maze.GetValue(X + x, Y + y) == 2)
            {
                Count++;
            }
        }
    }
    return Count < 1 && (Maze.GetValue(X, Y) == 1) ;
    return Count < 1 && (Maze.GetValue(X, Y) == 1) ;
}
//place the walls if its in the right position
void AMazeGenerator::PlaceRoomWalls(){
    for (int32 x = 1; x < Width - 1; x++)
    {
        for (int32 y = 1; y < Height - 1; y++)
        {
            if(IsWall(x, y)) { 
                PlacePiece(x, y, 90.f, WallPiece);
                 }
        }
    }
}
