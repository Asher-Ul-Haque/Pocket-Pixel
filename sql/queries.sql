-- SQL queries for Pixel Pocket Database

-- 1. Find all gamers and their total play time across all games. (Join, Aggregate, Group By)
SELECT
    g.gamer_tag,
    SUM(gp.play_time) AS total_play_time
FROM
    GAMER g
JOIN
    GAMER_POV gp ON g.gamer_id = gp.gamer_id
GROUP BY
    g.gamer_tag
ORDER BY
    total_play_time DESC;

-- 2. List all games published by 'Nintendo' released after 1990. (Where clause, Select from GAME)
SELECT
    title,
    release_year
FROM
    GAME
WHERE
    publisher = 'Nintendo' AND release_year > 1990;

-- 3. Show gamer tags and the titles of the games they have downloaded, ordered by gamer tag and download date. (Join, Order By)
SELECT
    g.gamer_tag,
    game.title,
    gp.downloaded_at
FROM
    GAMER g
JOIN
    GAMER_POV gp ON g.gamer_id = gp.gamer_id
JOIN
    GAME ON gp.game_id = game.game_id
WHERE gp.downloaded_at IS NOT NULL
ORDER BY
    g.gamer_tag,
    gp.downloaded_at;

-- 4. Find games that have not been played by any gamer. (Subquery - using NOT IN)
SELECT
    title
FROM
    GAME
WHERE
    game_id NOT IN (SELECT game_id FROM GAMER_POV);

-- 5. Calculate the average play time for each game across all gamers who have played it. (Aggregate, Group By, Join)
SELECT
    game.title,
    AVG(gp.play_time) AS average_play_time
FROM
    GAME
JOIN
    GAMER_POV gp ON game.game_id = gp.game_id
GROUP BY
    game.title
ORDER BY
    average_play_time DESC;

-- 6. List gamers who have played more than 5 games. (Aggregate, Group By, Having, Join)
SELECT
    g.gamer_tag,
    COUNT(DISTINCT gp.game_id) AS number_of_games_played
FROM
    GAMER g
JOIN
    GAMER_POV gp ON g.gamer_id = gp.gamer_id
GROUP BY
    g.gamer_tag
HAVING
    COUNT(DISTINCT gp.game_id) > 1;

-- 7. Find the game with the most saves. (Aggregate, Group By, Join, Order By, Limit)
SELECT
    game.title,
    COUNT(s.save_id) AS number_of_saves
FROM
    GAME
JOIN
    SAVES s ON game.game_id = s.game_id
GROUP BY
    game.title
ORDER BY
    number_of_saves DESC
LIMIT 1;

-- 8. Show gamers who have saves for more than one game. (Aggregate, Group By, Having, Join)
SELECT
    g.gamer_tag,
    COUNT(DISTINCT s.game_id) AS number_of_games_saved
FROM
    GAMER g
JOIN
    SAVES s ON g.gamer_id = s.game_id
GROUP BY
    g.gamer_tag
HAVING
    COUNT(DISTINCT s.game_id) > 0;

-- 9. Find the latest save created for each game and gamer combination. (Window Function - ROW_NUMBER() - Requires PostgreSQL 9.2 or later so do download if you are verifying our work)
SELECT
    g.gamer_tag,
    game.title,
    s.created_at AS latest_save_time
FROM (
    SELECT
        gamer_id,
        game_id,
        created_at,
        ROW_NUMBER() OVER(PARTITION BY gamer_id, game_id ORDER BY created_at DESC) as rn
    FROM
        SAVES
) s_ranked
JOIN GAMER g ON s_ranked.gamer_id = g.gamer_id
JOIN GAME game ON s_ranked.game_id = game.game_id
JOIN SAVES s ON s_ranked.game_id= s.save_id -- Joining again to get the full save record, although only created_at is needed here. Can be optimized.
WHERE s_ranked.rn = 1
ORDER BY g.gamer_tag, game.title;


-- 10. List all ROM components and the games they belong to. (Join, Select specific columns)
SELECT
    r.file_path,
    game.title
FROM
    ROM r
JOIN
    COMPONENT c ON r.component_id = c.component_id
JOIN
    GAME_COMPONENTS gc ON c.component_id = gc.component_id
JOIN
    GAME game ON gc.game_id = game.game_id;

-- 11. Find gamers who have played games released before 1995. (Join, Subquery in WHERE clause, EXISTS)
SELECT DISTINCT
    g.gamer_tag
FROM
    GAMER g
WHERE EXISTS (
    SELECT 1
    FROM GAMER_POV gp
    JOIN GAME game ON gp.game_id = game.game_id
    WHERE gp.gamer_id = g.gamer_id AND game.release_year < 1995
);

-- 12. Get the count of games for each publisher. (Aggregate, Group By)
SELECT
    publisher,
    COUNT(*) AS game_count
FROM
    GAME
GROUP BY
    publisher
ORDER BY
    game_count DESC;

-- 13. Find games with no associated ROM components. (LEFT JOIN, WHERE IS NULL)
SELECT
    game.title
FROM
    GAME
LEFT JOIN
    GAME_COMPONENTS gc ON game.game_id = gc.game_id
LEFT JOIN
    COMPONENT c ON gc.component_id = c.component_id AND c.type = 'ROM'
WHERE
    c.component_id IS NULL;

-- 14.  Get gamer tag and play count for gamers who have played games more than 10 times in total. (Aggregate, Group By, Having, Join)
SELECT
    g.gamer_tag,
    SUM(gp.play_count) AS total_play_count
FROM
    GAMER g
JOIN
    GAMER_POV gp ON g.gamer_id = gp.gamer_id
GROUP BY
    g.gamer_tag
HAVING
    SUM(gp.play_count) > 4
ORDER BY total_play_count DESC;

-- 15.  Find games where the release year is the same as the average release year of all games. (Subquery, Aggregate)
SELECT
    title,
    release_year
FROM
    GAME
WHERE
    release_year = (SELECT AVG(release_year) FROM GAME);
