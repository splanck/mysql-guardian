from flask_wtf import Form
from wtforms import StringField, PasswordField, SubmitField
from wtforms.validators import DataRequired

class SignupForm(Form):
    username = StringField ('Username')
    password = StringField ('Password')
    email = StringField('Email')
    submit = SubmitField ('Sign up')

class Loginform(Form):
    email = StringField('Email')
    password = PasswordField('Password')
    submit = SubmitField("Sign In")

class AddUser(Form):
    username = StringField ('Username')
    password = StringField ('Password')
    email = StringField('Email')
    submit = SubmitField ('Add the user')