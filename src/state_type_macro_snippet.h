// don't have any define guards for this snippet... it's meant to be used
// multiple times...

struct Last {};

#define etype(x) ST_##x,
typedef enum { STATES } StateType;
typedef std::vector<StateType> StateTypes;
#undef etype


#define etype(x) #x,
static const char *strState[] = { STATES };
inline const char *enum2str (StateType f) {
    return strState[static_cast<int>(f)];
}
#undef etype


#define etype(x) struct x {};
STATES
#undef etype



#define etype(x) template<> struct TypeToEnum<x> { inline static const StateType value = ST_##x; };
template<typename T> struct TypeToEnum {};
STATES
#undef etype


class EntityState {
  StateType _prevState;
  StateType _currentState;
  uint _updatedAt{0};
public:
  EntityState(StateType st) : _prevState(st), _currentState(st) {}
  EntityState() = default; // necessary for serialization.
  uint updatedAt() const { return _updatedAt; }
  StateType current() const { return _currentState; }
  StateType previous() const { return _prevState; }
  void revertToPreviousState(uint step) {
    assert(_currentState != _prevState);
    _updatedAt = step;
    std::swap(_currentState, _prevState);
  }
  uint stepsSinceUpdated(uint simStep) const {
    return simStep - _updatedAt;
  }
  bool isChangedThisStep(uint step) const {
    return _updatedAt == step;
  }
  bool hasChanged() const {
    return _currentState != _prevState;
  }
  void setNextState(StateType st, uint step) {
    _updatedAt = step;
    _prevState = _currentState;
    _currentState = st;
  }


  #define etype(x) x,
  inline static const auto stateTypeList = entt::type_list<
    STATES
    Last
  >{};
  #undef etype

  #define etype(x) case ST_##x: {registry.assign<x>(id);} break;
  static void assignTagByEnum(Registry& registry, StateType v, EntityType id) {
    switch (v) {
      STATES
    }
  }
  #undef etype


  #define etype(x) case ST_##x: {registry.remove<x>(id);} break;
  static void removeTagByEnum(Registry& registry, StateType v, EntityType id) {
    switch (v) {
      STATES
    }
  }
  #undef etype


  #define etype(x) if (registry.has<x>(id)) { \
    registry.remove<x>(id); \
  }
  static void removeAllTags(Registry& registry, EntityType id) {
    STATES
  }
  #undef etype

};

inline std::ostream& operator<<(std::ostream& os, const EntityState& state)
{
    os << enum2str(state.previous()) << " " << enum2str(state.current()) << " " << state.updatedAt();
    return os;
}

template<class DefaultState>
void setDefaultState(entt::prototype& proto) {
  proto.set<DefaultState>();
  proto.set<EntityState>(TypeToEnum<DefaultState>::value);
}
