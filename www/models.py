from flask_sqlalchemy import SQLAlchemy
from werkzeug.security import generate_password_hash, check_password_hash
db = SQLAlchemy()

class Guardian_user(db.Model):
  __tablename__ = 'users'
  id = db.Column(db.Integer, primary_key = True)
  username = db.Column(db.String(100))
  password = db.Column(db.String(100))
  admin = db.Column(db.Integer)
  email = db.Column(db.String(120))

  def __init__(self, username, email, password, admin):
    self.username = username 
    self.email = email
    self.password = password 
    self.admin = admin

class Guardian_servers(db.Model):
  __tablename__ = 'servers'
  id = db.Column(db.Integer, primary_key = True)
  hostname = db.Column(db.String(100))
  port = db.Column(db.Integer)
  username = db.Column(db.String(100))
  password = db.Column(db.String(100))

  def __init__(self, id, hostname, port, username, password):
    self.id = id
    self.hostname = hostname
    self.port = port
    self.username = username
    self.password = password 

class Backup_history(db.Model):
  __tablename__ = 'backup_history'
  id = db.Column(db.Integer, primary_key = True)
  server_id = db.Column(db.Integer)
  time = db.Column(db.Integer)
  db_name = db.Column(db.String(100))
  filename = db.Column(db.String(100))

  def __init__(self, id, server_id, time, db_name, filename):
    self.id = id
    self.server_id = server_id
    self.time = time
    self.db_name = db_name
    self.filename = filename



