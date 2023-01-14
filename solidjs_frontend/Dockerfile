FROM node:latest

WORKDIR /
RUN npm install -g solid-js babel-preset-solid degit
RUN npx degit solidjs/templates/ts-tailwindcss /app
WORKDIR /app
RUN npm i

