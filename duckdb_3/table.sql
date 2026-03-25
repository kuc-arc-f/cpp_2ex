
CREATE TABLE IF NOT EXISTS todos (
    title      TEXT NOT NULL,
    done       INTEGER NOT NULL DEFAULT 0,
    created_at TEXT
);  

CREATE SEQUENCE seq_todo_id START 1;
ALTER TABLE todos ADD COLUMN id INTEGER DEFAULT nextval('seq_todo_id');
