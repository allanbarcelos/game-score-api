# WebSocket Game Server

Português | [English](README.md)

Este projeto é um servidor WebSocket básico que permite o registro de usuários, a manutenção de suas pontuações e a consulta de pontuações para um jogo específico.

## Dependências

- **WebSocket++**: Biblioteca para lidar com conexões WebSocket.
- **nlohmann/json**: Biblioteca para lidar com JSON.
- **uuid**: Biblioteca para gerar UUIDs.

## Estrutura de Dados

- `User`: Representa um usuário com ID, game ID, WebSocket handle, username, score, level e timestamp de criação.
- `users`: Armazena todos os usuários registrados no servidor.
- `connection_list`: Armazena todas as conexões WebSocket ativas.

## Funções Auxiliares

- `generateUUID()`: Fornece a geração de ID único.
- `usernameExists`: Verifica se um nome de usuário já existe para um jogo específico.
- `compareUsers`: Fornece uma função de comparação para ordenar usuários por sua pontuação em ordem decrescente.
- `getScoresByGameID`: Retorna as maiores pontuações para um determinado game ID como um array JSON.

## Manipuladores WebSocket

- `on_message`: Lida com mensagens WebSocket recebidas. O servidor espera certos comandos/cargas úteis do cliente:
  - `/register <username> <game_id>`: Registra um usuário com o username e game_id fornecidos. Se bem-sucedido, o servidor envia o ID único do usuário de volta.
  - `/get-scores <game_id>`: Retorna as melhores pontuações para o game ID fornecido.
  - `/set-score <user_id>`: Aumenta a pontuação do usuário com o user_id fornecido em um e envia as pontuações atualizadas para todos os usuários conectados do mesmo jogo.

## Instruções de Uso

1. Compile o código usando um compilador C++ adequado com as bibliotecas mencionadas acima.
2. Execute o servidor. Ele começará a escutar na porta 9900 para conexões WebSocket.
3. Conecte-se ao servidor usando um cliente WebSocket e envie comandos conforme mencionado na seção de manipuladores.

## Observações

1. O código assume que a carga útil recebida para registrar um usuário estará no formato `/register <username> <game_id>`.
2. A pontuação do usuário é aumentada com cada solicitação `/set-score`. Não há tratamento de erro caso um user_id inválido seja fornecido.
3. Não há como definir o nível de um usuário, mesmo havendo um campo `level` na struct `User`.
4. O envio de atualizações de pontuação para todos os usuários de um jogo cada vez que a pontuação de um único usuário muda pode não escalar bem com um grande número de usuários.
5. Segurança e validação são mínimas. Em um cenário de produção, você desejará verificações mais rigorosas e tratamento de erros. Você também consideraria criptografar o tráfego WebSocket usando TLS (WSS).
6. O armazenamento de usuários em um vetor (`users`) funciona para este pequeno exemplo, mas para uma aplicação real, considere usar um banco de dados.

## Proximos passos

1. Substituir de bibliotecas de terceiros, como Websocket, por funcionalidades nativas de soquete para uma integração mais simplificada.
2. Aprimorar as medidas de segurança implementando políticas CORS mais rigorosas e validações robustas do lado do cliente.
3. Desenvolver técnicas eficientes de gerenciamento de memória para se proteger contra estouros de memória e possíveis injeções de memória.

## Contribuições

Sinta-se à vontade para contribuir com este projeto, adicionando mais funcionalidades, corrigindo bugs ou melhorando a documentação.
