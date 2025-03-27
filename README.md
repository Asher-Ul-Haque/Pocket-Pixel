<div align="center">
<img src="https://github.com/user-attachments/assets/635d3087-2efe-41a4-b063-ad29f683069b" alt="Logo" width="256" />

# Pocket-Pixel

### **Bored in college lectures?** *CO nahi ho raha?* Don't worry—Pocket-Pixel is here to keep you entertained! Relive the joy of retro gaming anytime, anywhere.

Pocket-Pixel is a Game Boy emulator designed to deliver a seamless gaming experience across Android and Web platforms. Save your progress, compete for high scores, and discover new games through smart recommendations. Dive into the nostalgia of Game Boy games with a modern twist!

![tribute](https://github.com/user-attachments/assets/d9cd301d-9125-4be8-8d02-6a16ac2af9e8)

</div>

---

## Features

### 1. Cross-Platform Gameplay
- **Android App**: Play Game Boy games on the go, optimized for touch controls.
- **Web App**: Enjoy gaming in your browser with WebAssembly for smooth performance.
- **Powerful Emulator Core**: Built in C/C++ ensuring high performance.

### 2. ROM Management
- **Server Integration**: Download popular ROMs directly within the Android app and play offline
- **Find Any ROM**: If it exists, it is here

### 3. Cloud Saves
- **Save Progress Anywhere**: Sync your game saves to a secure web server using PostgreSQL.
- **Pick Up Where You Left Off**: Switch seamlessly between devices without losing progress.

### 4. Competitive Gaming
- **Leaderboards**: Compete with others for high scores, hours played, and more.
- **Community Challenges**: Engage with global players in unique challenges.

### 5. Game Discovery
- **Smart Recommendations**: Discover new and popular games powered by machine learning.
- **Trending Titles**: Stay updated on what’s hot in the retro gaming community.

---

## Technology Stack

### Core Components
- **Emulator Core**: C/C++ for game emulation.
- **WebAssembly**: Bringing the emulator to the Web for a smooth in-browser experience.

### Android App
- **Kotlin Multiplatform (KMP)**: Native Android development with Kotlin for shared code.
- **JNI Integration**: Connecting the C++ emulator core to the Android app.

### Backend
- **Framework**: Spring Boot / Ktor for robust backend services.
- **Database**: PostgreSQL for storing user data, game saves, and leaderboards.

### Web App
- **Frontend**: Kotlin/JS for an interactive user experience.
- **WebAssembly Integration**: Running the emulator core directly in the browser.

---

## Repository Structure
```
pocket-pixel/
├── app/                    # The app for android, web and the C emulator Core
├── server/                 # Backend server (Spring Boot / Ktor) and PostGreSQL server
├── bin/                    # APK builds currently unstable
├── README.md               # Project documentation
└── LICENSE                 # License file
```

---

## Installation and Usage

### Android App (Soon on Google Play Store)
1. Clone the repository.
3. Build and run the app using Android Studio.

### Web App (Soon on vercel)
1. Clone the repository.
2. Navigate to the `wasmJSMain/` directory.
3. Build the WebAssembly module and run the frontend using your preferred server.

### Server Setup
1. Clone the repository.
2. Navigate to the `server/` directory.
3. Set up the PostgreSQL database.
4. Run the server using Spring Boot or Ktor.

---


## License
This project is licensed under the [MIT License](LICENSE).
The user is solely responsible for copyright breaking, from, and emulation laws. 
Nintendo shouldn't know this exists

---

**Get ready to turn boring lectures into epic gaming sessions with Pocket-Pixel!**

