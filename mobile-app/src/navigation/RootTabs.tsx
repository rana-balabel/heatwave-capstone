import { Ionicons } from '@expo/vector-icons';
import { createBottomTabNavigator } from '@react-navigation/bottom-tabs';

import { HistoryScreen } from '../screens/HistoryScreen';
import { HomeScreen } from '../screens/HomeScreen';
import { ProfileScreen } from '../screens/ProfileScreen';
// Weather tab disabled for now – uncomment when ready
// import { WeatherScreen } from '../screens/WeatherScreen';

export type RootTabsParamList = {
  Home: undefined;
  History: undefined;
  Profile: undefined;
  // Weather: undefined;
};

const Tab = createBottomTabNavigator<RootTabsParamList>();

export function RootTabs() {
  return (
    <Tab.Navigator
      screenOptions={({ route }) => ({
        headerTitleAlign: 'center',
        tabBarIcon: ({ color, size }) => {
          const name =
            route.name === 'Home'
              ? 'home'
              : route.name === 'History'
                ? 'time'
                : 'person'; // was: Profile | Weather (partly-sunny)

          return <Ionicons name={name} size={size} color={color} />;
        },
      })}
    >
      <Tab.Screen name="Home" component={HomeScreen} />
      <Tab.Screen name="History" component={HistoryScreen} />
      <Tab.Screen name="Profile" component={ProfileScreen} />
      {/* <Tab.Screen name="Weather" component={WeatherScreen} /> */}
    </Tab.Navigator>
  );
}

