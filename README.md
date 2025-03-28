
# CS 528 - Final Project

A server-client application that allows multiple remote users to connect and configure locally hosted Ollama LLMs to a private network wherein the LLMs can communicate in isolation amongst themselves.



## Build

Clone the repository:

```bash
  git clone git@github.com:JaredWinkens/CS_528_Final_Project.git
  cd CS_528_Final_Project
```
Generate the cmake files:

```bash
cmake --preset default
```

Build the project:

```bash
cmake --build --preset default
```

Resulting applications:

```bash
root/
├─ build/
│  ├─ client/
│  │  ├─ Chat_Client
│  ├─ server/
│  │  ├─ Chat_Server
```    
## Authors

- [Micheal Ballard](mailto:ballarmj@sunypoly.edu)
- [Anthony Dudinyak](mailto:duduinya@sunypoly.edu)
- [Declan Wayman](mailto:waymand@sunypoly.edu)
- [Jared Winkens](mailto:winkenj@sunpoly.edu)
