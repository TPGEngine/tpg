
// MuJoCo environment data
export interface Environment {
  id: number;
  name: string;
  image: string;
  description: string;
  complexity: 'beginner' | 'intermediate' | 'advanced';
}

export const environments: Environment[] = [
  { 
    id: 1, 
    name: 'Ant', 
    image: '/placeholder.svg',
    description: 'The Ant environment features a quadruped robot with the goal of learning to walk forward as fast as possible. This environment has a moderate action space and state space complexity, making it a good benchmark for reinforcement learning algorithms.',
    complexity: 'intermediate'
  },
  { 
    id: 2, 
    name: 'Half Cheetah', 
    image: '/placeholder.svg',
    description: 'The Half Cheetah environment involves a 2D cheetah robot that needs to learn to run forward. It has a continuous action space and requires the agent to learn coordination between multiple joints to achieve forward momentum.',
    complexity: 'intermediate'
  },
  { 
    id: 3, 
    name: 'Hopper', 
    image: '/placeholder.svg',
    description: 'In the Hopper environment, a one-legged robot must learn to hop forward without falling. This environment is simpler than others but still requires learning a stable hopping gait.',
    complexity: 'beginner'
  },
  { 
    id: 4, 
    name: 'Humanoid Standup', 
    image: '/placeholder.svg',
    description: 'The Humanoid Standup environment challenges an agent to control a humanoid robot to stand up from a lying position. This is a complex task with a high-dimensional state and action space.',
    complexity: 'advanced'
  },
  { 
    id: 5, 
    name: 'Inverted Pendulum', 
    image: '/placeholder.svg',
    description: 'The Inverted Pendulum is a classic control problem where the agent must balance a pole on a cart. It has a simple state and action space, making it a good starting point for RL algorithms.',
    complexity: 'beginner'
  },
  { 
    id: 6, 
    name: 'Inverted Double Pendulum', 
    image: '/placeholder.svg',
    description: 'The Inverted Double Pendulum extends the classic pendulum problem with two connected poles. The increased complexity makes it a more challenging control problem that requires sophisticated policies.',
    complexity: 'advanced'
  },
];

export const getEnvironmentById = (id: number): Environment | undefined => {
  return environments.find(env => env.id === id);
};
