type tPressure = {pressure: float};

type tListPressures = array(tPressure);

type state = {
  cityName: string,
  pressures: tListPressures
};

type action =
  | PressureLoaded(tListPressures)
  | UpdateCity(string)
  | LoadPressure;

let component = ReasonReact.reducerComponent("Page");

let handleClick = (_event, _self) => Js.log("clicked!");

type tStrangeType = {press: tListPressures};

module Decode = {
  let pressure = jsonPressure : tPressure => {
    pressure: jsonPressure |> Json.Decode.field("pressure", Json.Decode.float)
  };
  let listArray = json => json |> Json.Decode.array(pressure);
  let pressures = json : tListPressures =>
    Array.map(
      pressure => pressure,
      Json.Decode.{press: json |> field("list", listArray)}.press
    );
};

let calc = (t: tListPressures) : array(float) =>
  Array.mapi(
    (index, _item) =>
      switch index {
      | 0 => 0.0
      | _ => t[index].pressure -. t[index - 1].pressure
      },
    t
  );

let dropFirst = input =>
  if (Array.length(input) > 0) {
    Array.sub(input, 1, Array.length(input) - 1);
  } else {
    input;
  };

let indexToDate = (input: int) : string => {
  let indexToDayName = input => {
    let current = Js.Date.make();
    let dayWeek = Js.Date.getDay(current) |> int_of_float;
    let next = ref(dayWeek + input);
    Js.log(next^);
    if (next^ >= 7) {
      next := next^ - 7;
    } else {
      next := next^;
    };
    switch next^ {
    | 0 => "Sunday"
    | 1 => "Monday"
    | 2 => "Tuesday"
    | 3 => "Wednesday"
    | 4 => "Thursday"
    | 5 => "Friday"
    | 6 => "Saturday"
    | _ => "Ha"
    };
  };
  switch input {
  | 0 => "Today"
  | 1 => "Tomorrow"
  | _ => indexToDayName(input)
  };
};

let make = (_) => {
  ...component,
  initialState: () => {cityName: "Kyiv", pressures: [||]},
  reducer: (action, state) =>
    switch action {
    | UpdateCity(s) =>
      ReasonReact.UpdateWithSideEffects(
        {...state, cityName: s},
        (self => self.send(LoadPressure))
      )
    | PressureLoaded(pressures) =>
      ReasonReact.Update({pressures, cityName: state.cityName})
    | LoadPressure =>
      ReasonReact.SideEffects(
        (
          self =>
            Js.Promise.(
              Fetch.fetch(
                "http://api.openweathermap.org/data/2.5/forecast/daily?q="
                ++ state.cityName
                ++ "&cnt=7&mode=json&appid=e896545ab1632674c8cadbc58b500605"
              )
              |> then_(Fetch.Response.json)
              |> then_(json =>
                   json
                   |> Decode.pressures
                   |> (
                     pressures => {
                       self.send(PressureLoaded(pressures)) |> ignore;
                       resolve(pressures);
                     }
                   )
                 )
              |> ignore
            )
        )
      )
    },
  render: self =>
    <div>
      <Controls onCitySet=(newCity => self.send(UpdateCity(newCity))) />
      <div className="mui-container">
        (
          calc(self.state.pressures)
          |> dropFirst
          |> Array.mapi((index, pressure) =>
               <PressureItem
                 key=(string_of_int(index))
                 pressure
                 date=(indexToDate(index))
               />
             )
          |> ReasonReact.arrayToElement
        )
      </div>
    </div>
};
