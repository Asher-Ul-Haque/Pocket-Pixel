-- This file creates the database, tables etc in Pixel Pocket backend


--WARNING:: CREATE THE DATABASE AND CONNECT BEFORE STARTING
CREATE DATABASE pixel_pocket;   -- Create the database 
\c pixel_pocket               

-- NOTE: CREATE ALL THE TABLES

-- The Gamers
-- The rest of the group wants to call this Users, BUT CREATE TABLE GAMER             
-- GAMERS sounds 💅(I blame Asher for this). I am mentioning the names that the rest of the group will use anyways-- GAMERS sounds 💅. I am mentioning the names that the rest of the group will use anyways
CREATE TABLE GAMER              
(
  -- Attributes
  gamer_id      SERIAL,                                 -- user_id
  gamer_tag     VARCHAR(30) UNIQUE NOT NULL,            -- username
  password_hash TEXT        NOT NULL,                   -- stores the password but not exactly
  created_at    TIMESTAMP   DEFAULT CURRENT_TIMESTAMP,  -- Cant use Now() for some reason
  play_time     INTERVAL    DEFAULT '0',                -- How long has the user used the app. Gotta give google play store a reason to promote it
  play_count    INT         DEFAULT 0,                  -- How many games has the gamer played

  -- Keys
  PRIMARY KEY (gamer_id)
);

-- The Games I want to call this 💅 Cartridges, but I have been far less successful in convincing the grp of this
CREATE TABLE GAME           
(
  -- Attributes
  game_id       SERIAL,                                                     -- game_id
  title         VARCHAR(255)    NOT NULL,                                   -- title of the game eg: Pokemon Red
  release_year  INT             DEFAULT 1989 CHECK(release_year >= 1989),   -- Which year was the game released in. Obv cant be before 1989 (the launch year of Game Boy)
  publisher     VARCHAR(100),                                               -- Who made tha game? Capcop? Nintendo?
  box_art       BYTEA,                                                      -- A cover art for the game. 

  -- Keys 
  PRIMARY KEY (game_id)
);

-- Game from Gamer POV 
CREATE TABLE    GAMER_POV       -- The rest of the group wants to call this USERGAME, eww
(
  -- Attributes
  gamer_id          INT, 
  game_id           INT,
  downloaded_at     TIMESTAMP   DEFAULT NULL,   -- The user could simply play the game without downloading
  play_time         INTERVAL    DEFAULT '0',    -- How long has this particular game been played
  play_count        INT         DEFAULT 0,      -- How many times has this game been played

  -- Keys
  PRIMARY KEY (gamer_id, game_id),
  FOREIGN KEY (gamer_id)
    REFERENCES  GAMER(gamer_id)
    ON DELETE   CASCADE,
  FOREIGN KEY (game_id)
    REFERENCES  GAME(game_id)
    ON DELETE   CASCADE

);

-- SAVES TABLE (Stores user save states)
CREATE TABLE SAVES 
(
  -- Attributes
  save_id     SERIAL,
  gamer_id    INT,
  game_id     INT,
  slot_number INT       DEFAULT 1,                  -- Allows multiple save slots
  save_state  BYTEA     NOT NULL,                   -- The actual save file, we are still wondering, whether to store the entire file or file path
  created_at  TIMESTAMP DEFAULT CURRENT_TIMESTAMP,  -- In case , we have multiple saves

  -- Keys 
  PRIMARY KEY (save_id),
  FOREIGN KEY (gamer_id) 
    REFERENCES  GAMER(gamer_id) 
    ON DELETE   CASCADE,
  FOREIGN KEY (game_id) 
    REFERENCES  GAME(game_id) 
    ON DELETE   CASCADE
);

-- Components (This is like an abstract class)
CREATE TABLE COMPONENT 
(
  -- Attributes
  component_id SERIAL,
  type          CHAR(3) NOT NULL CHECK (type IN ('ROM', 'RAM')), -- What kind of component do we have

  -- Keys
  PRIMARY KEY (component_id)
);

-- ROM (The assembly instructions of the game) subtype of Components 
CREATE TABLE ROM 
(
  -- Attributes
  component_id  INT,        
  file_path     TEXT        NOT NULL,                   -- Where is the file located
  hash_sha256   TEXT        UNIQUE NOT NULL,            -- Make sure the game is genuine
  uploaded_at   TIMESTAMP   DEFAULT CURRENT_TIMESTAMP,  -- When was the game uploaded to the database

  -- Keys
  PRIMARY KEY (component_id),
  FOREIGN KEY (component_id)
    REFERENCES  COMPONENT(component_id)
    ON DELETE   CASCADE
);

-- RAM registers (state of the game)
CREATE TABLE RAM_REGISTER
(
  -- Attributes
  component_id  INT,                 
  register_file BYTEA   NOT NULL,       -- Might change in the future to one attribute per regisert
  save_id       INT     DEFAULT NULL,   -- A game might never have been saved, thus null, but if it is saved, load from the save file

  -- Keys 
  PRIMARY KEY (component_id),
  FOREIGN KEY (component_id)
    REFERENCES  COMPONENT(component_id)
    ON DELETE   CASCADE,
  FOREIGN KEY (save_id) 
    REFERENCES  SAVES(save_id)  
    ON DELETE   CASCADE
);

-- Game Components (links a game to its components)
CREATE TABLE GAME_COMPONENTS
(
  -- Attributes
  game_id       INT,                    -- Which game 
  component_id  INT,                    -- What component

  -- Keys 
  PRIMARY KEY (game_id, component_id),  -- both are required for the unique match
  FOREIGN KEY (game_id)
    REFERENCES  GAME(game_id)           
    ON DELETE   CASCADE,                -- Na rahe ga bans, na rahegi bansuri
  FOREIGN KEY (component_id)
    REFERENCES  COMPONENT(component_id)
    ON DELETE   CASCADE                 -- A Game cannot function with even one missing component
);
