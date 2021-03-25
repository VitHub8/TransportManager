
# TransportManager

В этой задаче нужно реализовать систему хранения транспортных маршрутов и обработки запросов к ней. На вход программе подаётся JSON-объект, описывающий запросы на создание базы данных (ключ base_requests) и запросы к этой базе (ключ stat_requests). Выход программы также ожидается в формате JSON.

Формат ввода базы данных
Запросы на создание объектов в базе данных задаются в виде списка в ключе base_requests входного JSON. Каждый запрос является словарём, содержащим ключ type — тип запроса, а также прочую дополнительную информацию о соответствующем объекте.
  Stop: описание остановки
  Bus: описание автобуса

Маршрут может задаваться в одном из двух форматов:
  "is_roundtrip": false: автобус следует по списку остановок из stops от первой до последней и обратно.
  "is_roundtrip": true: кольцевой маршрут, у которого первая остановка из списка stops является конечной.

Входной JSON содержит ключ routing_settings, значением которого является словарь с двумя ключами:
  "bus_wait_time" — время ожидания автобуса на остановке (в минутах). Считайте, что когда бы человек ни пришёл на остановку и какой бы ни была эта остановка, он будет ждать любой автобус в точности указанное количество минут. Значение — целое число от 1 до 1000.
  "bus_velocity" — скорость автобуса (в км/ч). Считайте, что скорость любого автобуса постоянна и в точности равна указанному числу. Время стоянки на остановках не учитывается, время разгона и торможения — тоже. Значение — вещественное число от 1 до 1000.
  
Запросы к базе данных задаются в виде списка в ключе stat_requests входного JSON. Каждый запрос является словарём, обязательно содержащим два ключа:
  type — тип запроса;
  id — целое число от 0 до 2147483647.
Ответы на запросы выводятся в виде списка, каждый элемент которого в поле request_id содержит id исходного запроса.
Типы запросов:
  Bus: информация об автобусе
  Stop: информация об остановке
  Route: построение маршрута между двумя остановками

Пример входного JSON-файла:

{
  "routing_settings": {
    "bus_wait_time": 6,
    "bus_velocity": 40
  },
  "base_requests": [
    {
      "type": "Bus",
      "name": "297",
      "stops": [
        "Biryulyovo Zapadnoye",
        "Biryulyovo Tovarnaya",
        "Universam",
        "Biryulyovo Zapadnoye"
      ],
      "is_roundtrip": true
    },
    {
      "type": "Bus",
      "name": "635",
      "stops": [
        "Biryulyovo Tovarnaya",
        "Universam",
        "Prazhskaya"
      ],
      "is_roundtrip": false
    },
    {
      "type": "Stop",
      "road_distances": {
        "Biryulyovo Tovarnaya": 2600
      },
      "longitude": 37.6517,
      "name": "Biryulyovo Zapadnoye",
      "latitude": 55.574371
    },
    {
      "type": "Stop",
      "road_distances": {
        "Prazhskaya": 4650,
        "Biryulyovo Tovarnaya": 1380,
        "Biryulyovo Zapadnoye": 2500
      },
      "longitude": 37.645687,
      "name": "Universam",
      "latitude": 55.587655
    },
    {
      "type": "Stop",
      "road_distances": {
        "Universam": 890
      },
      "longitude": 37.653656,
      "name": "Biryulyovo Tovarnaya",
      "latitude": 55.592028
    },
    {
      "type": "Stop",
      "road_distances": {},
      "longitude": 37.603938,
      "name": "Prazhskaya",
      "latitude": 55.611717
    }
  ],
  "stat_requests": [
    {
      "type": "Bus",
      "name": "297",
      "id": 1
    },
    {
      "type": "Bus",
      "name": "635",
      "id": 2
    },
    {
      "type": "Stop",
      "name": "Universam",
      "id": 3
    },
    {
      "type": "Route",
      "from": "Biryulyovo Zapadnoye",
      "to": "Universam",
      "id": 4
    },
    {
      "type": "Route",
      "from": "Biryulyovo Zapadnoye",
      "to": "Prazhskaya",
      "id": 5
    }
  ]
}

Пример JSON-файла на выходе:

[
    {
        "curvature": 1.42963,
        "unique_stop_count": 3,
        "stop_count": 4,
        "request_id": 1,
        "route_length": 5990
    },
    {
        "curvature": 1.30156,
        "unique_stop_count": 3,
        "stop_count": 5,
        "request_id": 2,
        "route_length": 11570
    },
    {
        "request_id": 3,
        "buses": [
            "297",
            "635"
        ]
    },
    {
        "total_time": 11.235,
        "items": [
            {
                "time": 6,
                "type": "Wait",
                "stop_name": "Biryulyovo Zapadnoye"
            },
            {
                "span_count": 2,
                "bus": "297",
                "type": "Bus",
                "time": 5.235
            }
        ],
        "request_id": 4
    },
    {
        "total_time": 24.21,
        "items": [
            {
                "time": 6,
                "type": "Wait",
                "stop_name": "Biryulyovo Zapadnoye"
            },
            {
                "span_count": 2,
                "bus": "297",
                "type": "Bus",
                "time": 5.235
            },
            {
                "time": 6,
                "type": "Wait",
                "stop_name": "Universam"
            },
            {
                "span_count": 1,
                "bus": "635",
                "type": "Bus",
                "time": 6.975
            }
        ],
        "request_id": 5
    }
]
