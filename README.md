Mythion-Game

A multiplayer action RPG built with Unreal Engine 5 and C++ — solo-developed as my Master's thesis project, focused on real-time gameplay systems and networked multiplayer architecture.

Overview

Mythion is a small-scale multiplayer online RPG. One developer, full stack: gameplay programming in Unreal Engine/C++ on the client, and a custom Node.js/TypeScript backend handling authentication, real-time communication, and persistence. The project was deliberately scoped for a small number of concurrent players (3–5) rather than true MMO scale, which shaped several of the design decisions below.

Features

Characters & Combat

Character selection between two playable classes: Mage and Warrior, each with a distinct combat kit.
Combat system built on Unreal's Gameplay Ability System (GAS) — abilities, attributes, and gameplay effects.
Real-time melee and ranged combat against multiple simultaneous enemies.

World

Leveling system that grants an extra life per level gained.
Respawn point system.
Quest system.
In-game shop.
Destructible objects.
Item and weapon pickups from the world, located via trace-based detection.

Multiplayer & Networking

Authoritative multiplayer architecture with a Node.js/TypeScript backend.
Authentication system.
Real-time communication over WebSockets.
Global (all-to-all) chat system.
Persistence via MongoDB.

UI

HUD widgets for health and mana.

Inventory

Custom inventory system with intentionally simple, non-optimized array-based serialization — a scoping decision appropriate for the project's target of 3–5 concurrent players, rather than a fully compressed/optimized serialization format built for large-scale MMO traffic.
