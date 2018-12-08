from flask_wtf import Form
from wtforms import StringField, PasswordField, SubmitField
from wtforms.validators import DataRequired

class SignupForm(Form):
    first_name = StringField ('First Name')
    last_name = StringField ('Last Name')
    email = StringField('Email')
    password = PasswordField('Password')
    submit = SubmitField ('Sign up')

class Loginform(Form):
    email = StringField('Email')
    password = PasswordField('Password')
    submit = SubmitField("Sign In")