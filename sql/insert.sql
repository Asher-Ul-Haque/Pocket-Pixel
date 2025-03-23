-- Sample Insert Script for Pocket-Pixel Database
-- Remember these are samples. This does not mean the other person cannot install something like *** sim.

-- Insert sample data into GAMER table (Users)
INSERT INTO GAMER (gamer_tag, password_hash, created_at, play_time, play_count) -- Asher came up with Gamers. Said it was 💅 (I have regrets)
VALUES 
  ('Sakshat123', 'hashed_password_1', '2025-02-15 10:00:00', '1 hour'::interval, 5),
  ('ParthG', 'hashed_password_2', '2025-02-10 09:30:00', '3 hours'::interval, 12),
  ('NischayK', 'hashed_password_3', '2025-02-12 11:15:00', '5 hours'::interval, 20),
  ('AsherH', 'hashed_password_4', '2025-02-11 14:00:00', '2 hours'::interval, 8);

-- Insert sample data into GAME table
-- SERIAL is auto I am assuming
INSERT INTO GAME (title, release_year, publisher, box_art)
VALUES 
  ('Pokemon Red', 1996, 'Nintendo', NULL),
  ('The Legend of Zelda', 1993, 'Nintendo', NULL), -- GOAT Game
  ('Tetris', 1989, 'Bullet-Proof Software', NULL), -- FAV Game
  ('Super Mario Land', 1989, 'Nintendo', NULL);

-- Insert sample data into GAMER_POV table (Linking GAMERs and GAMEs)
-- The name does not suit at all. Should have been USERGAME but no Asher had different plans
INSERT INTO GAMER_POV (gamer_id, game_id, downloaded_at, play_time, play_count)
VALUES 
  (1, 1, '2025-02-10 15:30:00', '30 minutes'::interval, 2),
  (1, 2, '2025-02-12 16:00:00', '1 hour'::interval, 3),
  (2, 3, '2025-02-13 14:00:00', '2 hours'::interval, 5),
  (3, 4, NULL, '45 minutes'::interval, 1);

-- Insert sample data into SAVES table (User Save States)
-- The byte file for now is a string. Need to learn how to make into byte file
INSERT INTO SAVES (gamer_id, game_id, slot_number, save_state, created_at)
VALUES 
  (1, 1, 1, '\x3fa5b8e3c7d91f2a', '2025-02-14 12:00:00'),
  (1, 2, 1, '\x3fa5b8e3c7d91f4a', '2025-02-14 13:00:00'),
  (2, 3, 1, '\x3fa5b8e3c7d91f3a', '2025-02-14 14:00:00');

-- Insert sample data into COMPONENT table
INSERT INTO COMPONENT (type)
VALUES 
  ('ROM'),
  ('ROM'),
  ('RAM');

-- Insert sample data into ROM table (Subtype of COMPONENT)
INSERT INTO ROM (component_id, file_path, hash_sha256, uploaded_at)
VALUES 
  (1, '/roms/pokemon_red.gbc', 'hash_value_1', '2025-02-10 10:00:00'), -- I am not sititng down and creating a hash function right now. Sorry not sorry
  (2, '/roms/zelda.gbc', 'hash_value_2', '2025-02-11 12:00:00');

-- Insert sample data into RAM_REGISTER table (Subtype of COMPONENT)
INSERT INTO RAM_REGISTER (component_id, register_file, save_id)
VALUES 
  (3, '\x3fa5b8e3c7d91f2a', 1);

-- Insert sample data into GAME_COMPONENTS table (Linking games with their components)
INSERT INTO GAME_COMPONENTS (game_id, component_id)
VALUES 
  (1, 1),  -- 'Pokemon Red' uses ROM component (component_id 1)
  (2, 2),  -- 'The Legend of Zelda' uses ROM component (component_id 2)
  (3, 3);  -- 'Tetris' uses RAM component (component_id 3)

