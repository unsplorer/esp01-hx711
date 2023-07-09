FROM node:latest

WORKDIR /app
RUN npm install -g solid-js babel-preset-solid degit
RUN npx degit solidjs/templates/ts-tailwindcss
COPY ./app /
CMD ["npm", "run", "dev"]