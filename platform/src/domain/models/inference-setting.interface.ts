export interface InferenceSetting {
  inferenceId: string;

  settings: InferenceSettingDocument[];
}

export interface InferenceSettingDocument {
  key: string;
  extensionName: string;
  title: string;
  description: string;
  controllerType: string;
  controllerProps: ControllerProps;
}

export interface ControllerProps {
  placeholder: string;
  value: string;
  type?: string;
}
